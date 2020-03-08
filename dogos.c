////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

extern struct FIFO8 KEYBOARD, MOUSE;

void enable_mouse(void);
void init_keyboard(void);

void DogOS_main(void) {
    init_idt();
    init_pic();
    io_sti();

    init_keyboard();

    init_palette();
    init_screen();

    static char cursor[256];
    init_mouse_cursor8(cursor);
    putblock8_8(160, 100, 16, 16, cursor);
    enable_mouse();

    char keybuf[32], mousebuf[128], s[4];
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
            dh = data / 16;
            dl = data % 16;
            s[0] = (dh < 10)?(dh + 0x30):(dh + 0x37);
            s[1] = (dl < 10)?(dl + 0x30):(dl + 0x37);
            s[2] = 0;
            boxfill8(32, 16, 47, 31, COL8_008484);
            putfonts8_asc(32, 16, COL8_FFFFFF, s);
        }
        
        io_stihlt();
    }
}

#define PORT_KEYDAT             0x0060
#define PORT_KEYSTA             0x0064
#define PORT_KEYCMD             0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE       0x60
#define KBC_MODE                0x47

void wait_KBC_sendready(void) {
    while(1) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) break;
	}
}

void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4

void enable_mouse(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}
