////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

extern struct FIFO8 KEYBOARD, MOUSE;
extern struct MEMMAN MEMORY;

void c2hex(char *s, unsigned char data);

void DogOS_main(void) {
    // 开放中断
    init_idt();
    init_pic();
    io_sti();

    // 初始化图形界面
    init_palette();
    init_screen();

    // 初始化键盘、鼠标以及数据缓冲区
    init_keyboard();
    static char cursor[256];
    init_mouse_cursor8(cursor);
    int mx = 160, my = 100;
    putblock8_8(mx, my, 16, 16, cursor);
    struct MOUSE_DEC mdec;
    enable_mouse(&mdec);

    char keybuf[32], mousebuf[128], s[12];
    fifo8_init(&KEYBOARD, 32, keybuf);
    fifo8_init(&MOUSE, 128, mousebuf);
    unsigned char data;

    // 初始0x00200000以上内存可用
    memman_init(&MEMORY);
    unsigned int memtotal = memtest(0x00400000, 0xbfffffff);
    memman_free(&MEMORY, 0x00200000, memtotal - 0x00200000);

    data = memtotal / (1024 * 1024);
    c2hex(s, data);
    s[2] = 'M';
    s[3] = 'B';
    s[4] = 0;
    putfonts8_asc(0, 0, COL8_FFFFFF, s);

    // 鼠标键盘交互
    for(;;) {
        io_hlt();

        while(fifo8_status(&KEYBOARD)) {
            io_cli();
            data = fifo8_get(&KEYBOARD);
            io_sti();
            c2hex(s, data);
            boxfill8(0, 16, 15, 31, COL8_008484);
            putfonts8_asc(0, 16, COL8_FFFFFF, s);
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
                boxfill8(32, 16, 31+8*11, 31, COL8_008484);
                putfonts8_asc(32, 16, COL8_FFFFFF, s);
                boxfill8(mx, my, mx + 15, my + 15, COL8_008484);
                mx += mdec.x;
                my += mdec.y;
                if(mx < 0) mx = 0;
                if(my < 0) my = 0;
                if(mx > 320 - 16) mx = 320 - 16;
                if(my > 200 - 16) my = 200 - 16;
                putblock8_8(mx, my, 16, 16, cursor);
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
