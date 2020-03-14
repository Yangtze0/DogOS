////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

extern struct FIFO8 KEYBOARD, MOUSE;
extern struct MEMMAN MEMORY;
extern struct SHEETCTL SHEETS;
extern struct TIMERCTL TIMERS;

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void c2hex(char *s, unsigned char data);

void DogOS_main(void) {
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
    shtctl_init(&SHEETS);

    unsigned char *buf_back = (unsigned char *)memman_alloc_4k(320 * 200);
    init_screen(buf_back);
    struct SHEET *sht_back = sheet_alloc(0, 0, 320, 200, buf_back);
    sheet_updown(sht_back, 1);              // 背景图层

    unsigned char *buf_win = (unsigned char *)memman_alloc_4k(160 * 52);
    make_window8(buf_win, 160, 52, "Counter");
    struct SHEET *sht_win = sheet_alloc(80, 72, 160, 52, buf_win);
    sheet_updown(sht_win, 2);               // 窗口图层

    unsigned char buf_mouse[16 * 16];
    init_mouse_cursor8(buf_mouse);
    int mx = 160, my = 100;
    struct SHEET *sht_mouse = sheet_alloc(mx, my, 16, 16, buf_mouse);
    sheet_updown(sht_mouse, 3);             // 鼠标图层

    // 内存容量测试，结果显示在背景图层
    unsigned char data = memtotal / (1024 * 1024);
    char s[12];
    c2hex(s, data);
    s[2] = 'M';
    s[3] = 'B';
    s[4] = 0;
    putfonts8_asc(buf_back, 320, 0, 0, COL8_FFFFFF, s);
    sheet_refresh(sht_back, 0, 0, 8*4, 16);

    // 定时器
    struct FIFO8 timerfifo[3];
    char timerbuf[3][8];
    struct TIMER *timer[3];
    for(int i = 0; i < 3; i++) {
        fifo8_init(&timerfifo[i], 8, timerbuf[i]);
        timer[i] = timer_alloc();
        timer_init(timer[i], &timerfifo[i], 1);
    }
    timer_settime(timer[0], 1000);
    timer_settime(timer[1], 300);
    timer_settime(timer[2], 50);


    for(;;) {
        // 计数器，结果显示在窗口图层
        for(int i = 0; i < 4; i++) {
            data = ((unsigned char *)&TIMERS.count)[3-i];
            c2hex(s+(2*i), data);
        }
        s[8] = 0;
        boxfill8(buf_win, 160, 40, 28, 39+8*8, 43, COL8_C6C6C6);
        putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
        sheet_refresh(sht_win, 40, 28, 39+8*8, 43);

        io_hlt();

        // 键盘中断响应
        while(fifo8_status(&KEYBOARD)) {
            io_cli();
            data = fifo8_get(&KEYBOARD);
            io_sti();
            c2hex(s, data);
            boxfill8(buf_back, 320, 0, 16, 15, 31, COL8_008484);
            putfonts8_asc(buf_back, 320, 0, 16, COL8_FFFFFF, s);
            sheet_refresh(sht_back, 0, 16, 15, 31);
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
                boxfill8(buf_back, 320, 32, 16, 31+8*11, 31, COL8_008484);
                putfonts8_asc(buf_back, 320, 32, 16, COL8_FFFFFF, s);
                sheet_refresh(sht_back, 32, 16, 31+8*11, 31);
                mx += mdec.x;
                my += mdec.y;
                if(mx < 0) mx = 0;
                if(my < 0) my = 0;
                if(mx > 320 - 1) mx = 320 - 1;
                if(my > 200 - 1) my = 200 - 1;
                sheet_slide(sht_mouse, mx, my);
            }
        }

        // 定时器1超时响应
        while(fifo8_status(&timerfifo[0])) {
            io_cli();
            data = fifo8_get(&timerfifo[0]);
            io_sti();
            putfonts8_asc(buf_back, 320, 0, 32, COL8_FFFFFF, "10[sec]");
            sheet_refresh(sht_back, 0, 32, 56, 48);
        }

        // 定时器2超时响应
        while(fifo8_status(&timerfifo[1])) {
            io_cli();
            data = fifo8_get(&timerfifo[1]);
            io_sti();
            putfonts8_asc(buf_back, 320, 0, 48, COL8_FFFFFF, "3[sec]");
            sheet_refresh(sht_back, 0, 48, 48, 64);
        }

        // 定时器3超时响应，模拟光标
        while(fifo8_status(&timerfifo[2])) {
            io_cli();
            data = fifo8_get(&timerfifo[2]);
            io_sti();
            if (data) {
                timer[2]->data = 0;
                boxfill8(buf_back, 320, 0, 64, 7, 80, COL8_FFFFFF);
            } else {
                timer[2]->data = 1;
                boxfill8(buf_back, 320, 0, 64, 7, 80, COL8_008484);
            }
            timer_settime(timer[2], 50);
            sheet_refresh(sht_back, 0, 64, 7, 80);
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
