////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

extern struct FIFO8 KEYBOARD, MOUSE;

void DogOS_main(void) {
    init_idt();
    init_pic();
    io_sti();

    init_keyboard();
    init_palette();
    init_screen();

    static char cursor[256];
    init_mouse_cursor8(cursor);
    int mx = 160, my = 100;
    putblock8_8(mx, my, 16, 16, cursor);
    struct MOUSE_DEC mdec;
    enable_mouse(&mdec);

    char keybuf[32], mousebuf[128], s[12];
    fifo8_init(&KEYBOARD, 32, keybuf);
    fifo8_init(&MOUSE, 128, mousebuf);
    unsigned char data, dh, dl;

    for(;;) {
        io_cli();

        while(fifo8_status(&KEYBOARD)) {
            data = fifo8_get(&KEYBOARD);
            io_sti();
            dh = data / 16;
            dl = data % 16;
            s[0] = (dh < 10)?(dh + 0x30):(dh + 0x37);
            s[1] = (dl < 10)?(dl + 0x30):(dl + 0x37);
            s[2] = 0;
            boxfill8(0, 16, 15, 31, COL8_008484);
            putfonts8_asc(0, 16, COL8_FFFFFF, s);
        }

        while(fifo8_status(&MOUSE)) {
            data = fifo8_get(&MOUSE);
            io_sti();
            if(mouse_decode(&mdec, data)) {
                s[0] = '[';
                s[1] = (mdec.btn & 0x01)?'L':'l';
                s[2] = (mdec.btn & 0x02)?'C':'c';
                s[3] = (mdec.btn & 0x04)?'R':'r';
                s[4] = ' ';
                dh = mdec.x / 16;
                dl = mdec.x % 16;
                s[5] = (dh < 10)?(dh + 0x30):(dh + 0x37);
                s[6] = (dl < 10)?(dl + 0x30):(dl + 0x37);
                s[7] = ' ';
                dh = mdec.y / 16;
                dl = mdec.y % 16;
                s[8] = (dh < 10)?(dh + 0x30):(dh + 0x37);
                s[9] = (dl < 10)?(dl + 0x30):(dl + 0x37);
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
        
        io_stihlt();
    }
}


