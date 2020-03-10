////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

extern struct FIFO8 KEYBOARD, MOUSE;
extern struct MEMMAN MEMORY;
extern struct SHTCTL SHEETS;


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

    unsigned char buf_mouse[16 * 16];
    init_mouse_cursor8(buf_mouse);
    int mx = 160, my = 100;
    struct SHEET *sht_mouse = sheet_alloc(mx, my, 16, 16, buf_mouse);
    sheet_updown(sht_mouse, 2);             // 鼠标图层

    unsigned char data = memtotal / (1024 * 1024);
    char s[12];
    c2hex(s, data);
    s[2] = 'M';
    s[3] = 'B';
    s[4] = 0;
    putfonts8_asc(buf_back, 0, 0, COL8_FFFFFF, s);
    sheet_refresh(sht_back);

    // 键盘、鼠标响应
    for(;;) {
        io_hlt();

        while(fifo8_status(&KEYBOARD)) {
            io_cli();
            data = fifo8_get(&KEYBOARD);
            io_sti();
            c2hex(s, data);
            boxfill8(buf_back, 0, 16, 15, 31, COL8_008484);
            putfonts8_asc(buf_back, 0, 16, COL8_FFFFFF, s);
            sheet_refreshsub(0, 16, 15, 31);
        }

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
                boxfill8(buf_back, 32, 16, 31+8*11, 31, COL8_008484);
                putfonts8_asc(buf_back, 32, 16, COL8_FFFFFF, s);
                sheet_refreshsub(32, 16, 31+8*11, 31);
                mx += mdec.x;
                my += mdec.y;
                if(mx < 0) mx = 0;
                if(my < 0) my = 0;
                if(mx > 320 - 16) mx = 320 - 16;
                if(my > 200 - 16) my = 200 - 16;
                sheet_slide(sht_mouse, mx, my);
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
