
#include "dogos.h"

extern char FONT[4096];
unsigned char *VRAM = (unsigned char *)0xa0000; //  VGA320*200*8

void init_palette(void) {
    static unsigned char table_rgb[16*3] = {
        0x00, 0x00, 0x00,   /*  0:  黑      */
        0xff, 0x00, 0x00,   /*  1:  亮红    */
        0x00, 0xff, 0x00,   /*  2:  亮绿    */
        0xff, 0xff, 0x00,   /*  3:  亮黄    */
        0x00, 0x00, 0xff,   /*  4:  亮蓝    */
        0xff, 0x00, 0xff,   /*  5:  亮紫    */
        0x00, 0xff, 0xff,   /*  6:  浅亮蓝  */
        0xff, 0xff, 0xff,   /*  7:  白      */
        0xc6, 0xc6, 0xc6,   /*  8:  亮灰    */
        0x84, 0x00, 0x00,   /*  9:  暗红    */
        0x00, 0x84, 0x00,   /*  10: 暗绿    */
        0x84, 0x84, 0x00,   /*  11: 暗黄    */
        0x00, 0x00, 0x84,   /*  12: 暗青    */
        0x84, 0x00, 0x84,   /*  13: 暗紫    */
        0x00, 0x84, 0x84,   /*  14: 浅暗蓝  */
        0x84, 0x84, 0x84    /*  15: 暗灰    */
    };

    int eflags = io_load_eflags();
    io_cli();
    for(int i = 0; i <= 15; i++) {
        io_out8(0x03c8, i);
        io_out8(0x03c9, table_rgb[i*3+0]/4);
        io_out8(0x03c9, table_rgb[i*3+1]/4);
        io_out8(0x03c9, table_rgb[i*3+2]/4);
    }
    io_store_eflags(eflags);
}

void boxfill8(unsigned char *vram, int xs, int x0, int y0, int x1, int y1, unsigned char color) {
    for (int y = y0; y<= y1; y++) {
        for (int x = x0; x <= x1; x++)
            vram[y * xs + x] = color;
    }
}

void init_screen(unsigned char *vram) {
    int xm = 320, ym = 200;

    boxfill8(vram, xm, 0,     0,      xm-1,   ym-29,  COL8_008484);
    boxfill8(vram, xm, 0,     ym-28,  xm-1,   ym-28,  COL8_C6C6C6);
    boxfill8(vram, xm, 0,     ym-27,  xm-1,   ym-27,  COL8_FFFFFF);
    boxfill8(vram, xm, 0,     ym-26,  xm-1,   ym-1,   COL8_C6C6C6);

    boxfill8(vram, xm, 3,     ym-24,  59,     ym-24,  COL8_FFFFFF);
    boxfill8(vram, xm, 2,     ym-24,  2,      ym-4,   COL8_FFFFFF);
    boxfill8(vram, xm, 3,     ym-4,   59,     ym-4,   COL8_848484);
    boxfill8(vram, xm, 59,    ym-23,  59,     ym-5,   COL8_848484);
    boxfill8(vram, xm, 2,     ym-3,   59,     ym-3,   COL8_000000);
    boxfill8(vram, xm, 60,    ym-24,  60,     ym-3,   COL8_000000);

    boxfill8(vram, xm, xm-47, ym-24,  xm-4,   ym-24,  COL8_848484);
    boxfill8(vram, xm, xm-47, ym-23,  xm-47,  ym-4,   COL8_848484);
    boxfill8(vram, xm, xm-47, ym-3,   xm-4,   ym-3,   COL8_FFFFFF);
    boxfill8(vram, xm, xm-3,  ym-24,  xm-3,   ym-3,   COL8_FFFFFF);
}

void putfont8(unsigned char *vram, int xs, int x, int y, unsigned char color, char *font) {
    unsigned char *row;
    char data;
    for(int i = 0; i < 16; i++) {
        row = vram + (y + i) * xs + x;
		data = font[i];
        for(int j = 0; j < 8; j++) {
            if(data & (0x80 >> j)) row[j] = color;
        }
    }
}

void putfonts8_asc(unsigned char *vram, int xs, int x, int y, unsigned char color, char *s) {
    while (*s) {
        putfont8(vram, xs, x, y, color, FONT + (*s) * 16);
        x += 8;
        s++;
    }
}

void init_mouse_cursor8(unsigned char *mouse) {
    static char cursor[16][16] = {
        "**************..",
        "*OOOOOOOOOOO*...",
        "*OOOOOOOOOO*....",
        "*OOOOOOOOO*.....",
        "*OOOOOOOO*......",
        "*OOOOOOO*.......",
        "*OOOOOOO*.......",
        "*OOOOOOOO*......",
        "*OOOO**OOO*.....",
        "*OOO*..*OOO*....",
        "*OO*....*OOO*...",
        "*O*......*OOO*..",
        "**........*OOO*.",
        "*..........*OOO*",
        "............*OO*",
        ".............***"
    };

	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
            switch (cursor[y][x])
            {
            case '*':
                mouse[y * 16 + x] = COL8_000000;
                break;
            case 'O':
                mouse[y * 16 + x] = COL8_FFFFFF;
                break;
            case '.':
                mouse[y * 16 + x] = COL_INVISIBLE;
                break;
            }
		}
	}
}

void putblock8_8(unsigned char *vram, int x0, int y0, int sx, int sy, char *mouse) {
	for (int y = 0; y < sy; y++) {
		for (int x = 0; x < sx; x++) {
			vram[(y0 + y) * 320 + (x0 + x)] = mouse[y * sx + x];
		}
	}
}
