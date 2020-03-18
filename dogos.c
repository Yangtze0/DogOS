////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

extern struct FIFO8 KEYBOARD, MOUSE;
extern struct MEMMAN MEMORY;
extern struct SHEETCTL SHEETS;
extern struct TIMERCTL TIMERS;

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int b, int c, char *s, int l);
void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void c2hex(char *s, unsigned char data);

void DogOS_main(void) {
    // 系统信息
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

    // 开放中断
    init_idt();
    init_pic();
    io_sti();

    // 初始化键盘、鼠标以及数据缓冲区
    init_keyboard();
    struct MOUSE_DEC mdec;
    enable_mouse(&mdec);
    char keybuf[32], mousebuf[128];
    fifo8_init(&KEYBOARD, 32, keybuf);
    fifo8_init(&MOUSE, 128, mousebuf);

    // 初始0x00200000以上内存可用
    memman_init(&MEMORY);
    unsigned int memtotal = memtest(0x00400000, 0xbfffffff);
    memman_free(0x00200000, memtotal - 0x00200000);

    // 初始化图形界面
    init_palette();
    shtctl_init(&SHEETS, binfo->vram, binfo->scrnx, binfo->scrny);
    unsigned char *buf_back = (unsigned char *)memman_alloc_4k(binfo->scrnx * binfo->scrny);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    struct SHEET *sht_back = sheet_alloc(0, 0, binfo->scrnx, binfo->scrny, buf_back);
    sheet_updown(sht_back, 1);              // 背景图层

    unsigned char *buf_win = (unsigned char *)memman_alloc_4k(160 * 52);
    make_window8(buf_win, 160, 52, "Window");
    struct SHEET *sht_win = sheet_alloc(80, 72, 160, 52, buf_win);
    make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
    sheet_updown(sht_win, 2);               // 窗口图层

    unsigned char buf_mouse[16 * 16];
    init_mouse_cursor8(buf_mouse);
    int mx = binfo->scrnx/2, my = binfo->scrny/2;
    struct SHEET *sht_mouse = sheet_alloc(mx, my, 16, 16, buf_mouse);
    sheet_updown(sht_mouse, 3);             // 鼠标图层

    // 内存容量测试，结果显示在背景图层
    unsigned char data = memtotal / (1024 * 1024);
    char s[12];
    c2hex(s, data);
    s[2] = 'M';
    s[3] = 'B';
    s[4] = 0;
    putfonts8_asc_sht(sht_back, 0, 0, COL_INVISIBLE, COL8_FFFFFF, s, 4);

    // 定时器
    struct FIFO8 timerfifo;
    char timerbuf[8];
    fifo8_init(&timerfifo, 8, timerbuf);
    struct TIMER *timer1, *timer2, *timer3;
    timer1 = timer_alloc();
    timer_init(timer1, &timerfifo, 10);
    timer_settime(timer1, 1000);
    timer2 = timer_alloc();
    timer_init(timer2, &timerfifo, 3);
    timer_settime(timer2, 300);
    timer3 = timer_alloc();
    timer_init(timer3, &timerfifo, 1);
    timer_settime(timer3, 50);

    static char keytable[0x54] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.'
	};
    int cursor_x = 8, cursor_c = COL8_FFFFFF;

    for(;;) {
        io_hlt();

        // 键盘中断响应
        while(fifo8_status(&KEYBOARD)) {
            io_cli();
            data = fifo8_get(&KEYBOARD);
            io_sti();
            c2hex(s, data);
            putfonts8_asc_sht(sht_back, 0, 16, COL8_008484, COL8_FFFFFF, s, 2);

            // 可显示字符
            if(data < 0x54 && keytable[data] && cursor_x < 144) {
                s[0] = keytable[data];
                s[1] = 0;
                putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
                cursor_x += 8;
            }

            // 退格键
            if(data == 0x0e && cursor_x > 8) {
                putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
                cursor_x -= 8;
            }

            // 光标再显示
            boxfill8(sht_win->buf, sht_win->xs, cursor_x, 28, cursor_x+7, 43, cursor_c);
            sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
        }

        // 鼠标中断响应
        while(fifo8_status(&MOUSE)) {
            io_cli();
            data = fifo8_get(&MOUSE);
            io_sti();
            if(mouse_decode(&mdec, data)) {
                s[0] = '[';
                s[1] = (mdec.btn & 0x01)?'L':'l';
                s[2] = (mdec.btn & 0x02)?'C':'c';
                s[3] = (mdec.btn & 0x04)?'R':'r';
                s[4] = ' ';
                c2hex(&s[5], mdec.x);
                s[7] = ' ';
                c2hex(&s[8], mdec.y);
                s[10] = ']';
                s[11] = 0;
                putfonts8_asc_sht(sht_back, 32, 16, COL8_008484, COL8_FFFFFF, s, 11);
                mx += mdec.x;
                my += mdec.y;
                if(mx < 0) mx = 0;
                if(my < 0) my = 0;
                if(mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                if(my > binfo->scrny - 1) my = binfo->scrny - 1;
                sheet_slide(sht_mouse, mx, my);

                // 按下左键，移动窗口
                if(mdec.btn & 0x01) sheet_slide(sht_win, mx-80, my-8);
            }
        }

        // 定时器超时响应
        while(fifo8_status(&timerfifo)) {
            io_cli();
            data = fifo8_get(&timerfifo);
            io_sti();
            switch (data) {
            case 10:
                putfonts8_asc_sht(sht_back, 0, 32, COL_INVISIBLE, COL8_FFFFFF, "10[sec]", 7);
                break;
            case 3:
                putfonts8_asc_sht(sht_back, 0, 48, COL_INVISIBLE, COL8_FFFFFF, "3[sec]", 6);
                break;
            case 1:
            case 0:
                if (data) {
                    timer3->data = 0;
                    cursor_c = COL8_000000;
                } else {
                    timer3->data = 1;
                    cursor_c = COL8_FFFFFF;
                }
                timer_settime(timer3, 50);
                boxfill8(sht_win->buf, sht_win->xs, cursor_x, 28, cursor_x+7, 43, cursor_c);
                sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
                break;
            }
        }
    }
}

void c2hex(char *s, unsigned char data) {
    unsigned char dh, dl;
    dh = data / 16;
    dl = data % 16;
    s[0] = (dh < 10)?(dh + 0x30):(dh + 0x37);
    s[1] = (dl < 10)?(dl + 0x30):(dl + 0x37);
    s[2] = 0;
}

void make_window8(unsigned char *buf, int xs, int ys, char *title) {
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };

    boxfill8(buf, xs, 0,    0,      xs-1,   0,      COL8_C6C6C6);
    boxfill8(buf, xs, 1,    1,      xs-2,   1,      COL8_FFFFFF);
    boxfill8(buf, xs, 0,    0,      0,      ys-1,   COL8_C6C6C6);
    boxfill8(buf, xs, 1,    1,      1,      ys-2,   COL8_FFFFFF);
    boxfill8(buf, xs, xs-2, 1,      xs-2,   ys-2,   COL8_848484);
    boxfill8(buf, xs, xs-1, 0,      xs-1,   ys-1,   COL8_000000);
    boxfill8(buf, xs, 2,    2,      xs-3,   ys-3,   COL8_C6C6C6);
    boxfill8(buf, xs, 3,    3,      xs-4,   20,     COL8_000084);
    boxfill8(buf, xs, 1,    ys-2,   xs-2,   ys-2,   COL8_848484);
    boxfill8(buf, xs, 0,    ys-1,   xs-1,   ys-1,   COL8_000000);
    putfonts8_asc(buf, xs, 24, 4, COL8_FFFFFF, title);

    char c;
    for (int y = 0; y < 14; y++) {
        for (int x = 0; x < 16; x++) {
            c = closebtn[y][x];
            switch (c)
            {
            case '@':
                c = COL8_000000;
                break;
            case '$':
                c = COL8_848484;
                break;
            case 'Q':
                c = COL8_C6C6C6;
                break;
            default:
                c = COL8_FFFFFF;
                break;
            }
            buf[(5 + y) * xs + (xs - 21 + x)] = c;
        }
    }
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int b, int c, char *s, int l) {
    boxfill8(sht->buf, sht->xs, x, y, x + l * 8 - 1, y + 15, b);
    putfonts8_asc(sht->buf, sht->xs, x, y, c, s);
    sheet_refresh(sht, x, y, x + l * 8, y + 16);
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c) {
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->xs, x0 - 2, y0 - 3, x1 + 1, y0 - 3, COL8_848484);
	boxfill8(sht->buf, sht->xs, x0 - 3, y0 - 3, x0 - 3, y1 + 1, COL8_848484);
	boxfill8(sht->buf, sht->xs, x0 - 3, y1 + 2, x1 + 1, y1 + 2, COL8_FFFFFF);
	boxfill8(sht->buf, sht->xs, x1 + 2, y0 - 3, x1 + 2, y1 + 2, COL8_FFFFFF);
	boxfill8(sht->buf, sht->xs, x0 - 1, y0 - 2, x1 + 0, y0 - 2, COL8_000000);
	boxfill8(sht->buf, sht->xs, x0 - 2, y0 - 2, x0 - 2, y1 + 0, COL8_000000);
	boxfill8(sht->buf, sht->xs, x0 - 2, y1 + 1, x1 + 0, y1 + 1, COL8_C6C6C6);
	boxfill8(sht->buf, sht->xs, x1 + 1, y0 - 2, x1 + 1, y1 + 1, COL8_C6C6C6);
	boxfill8(sht->buf, sht->xs, x0 - 1, y0 - 1, x1 + 0, y1 + 0, c);
}
