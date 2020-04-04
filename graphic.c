
#include "dogos.h"

extern char FONT[4096];

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

void boxfill(unsigned char *buf, int xs, int bx0, int by0, int bx1, int by1, unsigned char c) {
    for (int by = by0; by < by1; by++) {
        for (int bx = bx0; bx < bx1; bx++) {
            if(c != COL_INVISIBLE) buf[by * xs + bx] = c;
        }
    }
}

void drawline(unsigned char *buf, int xs, int bx0, int by0, int bx1, int by1, unsigned char c) {
    int dx = bx1 - bx0, dy = by1 - by0;
    int x = bx0 << 10, y = by0 << 10;
    int len;

    if (dx < 0) dx = - dx;
    if (dy < 0) dy = - dy;
	if (dx >= dy) {
        len = dx + 1;

        if (bx0 > bx1) dx = -1024;
        else dx =  1024;

        if (by0 <= by1) dy = ((by1 - by0 + 1) << 10) / len;
        else dy = ((by1 - by0 - 1) << 10) / len;
    } else {
        len = dy + 1;

        if (by0 > by1) dy = -1024;
        else dy =  1024;

        if (bx0 <= bx1) dx = ((bx1 - bx0 + 1) << 10) / len;
        else dx = ((bx1 - bx0 - 1) << 10) / len;
    }

    for (int i = 0; i < len; i++) {
        buf[(y >> 10) * xs + (x >> 10)] = c;
        x += dx;
        y += dy;
    }
}

void init_screen(unsigned char *buf, int xs, int ys) {
    boxfill(buf, xs, 0,     0,     xs,     ys-28,  COL8_008484);
    boxfill(buf, xs, 0,     ys-28, xs,     ys-27,  COL8_C6C6C6);
    boxfill(buf, xs, 0,     ys-27, xs,     ys-26,  COL8_FFFFFF);
    boxfill(buf, xs, 0,     ys-26, xs,     ys,     COL8_C6C6C6);

    boxfill(buf, xs, 3,     ys-24, 60,     ys-23,  COL8_FFFFFF);
    boxfill(buf, xs, 2,     ys-24, 3,      ys-3,   COL8_FFFFFF);
    boxfill(buf, xs, 3,     ys-4,  60,     ys-3,   COL8_848484);
    boxfill(buf, xs, 59,    ys-23, 60,     ys-4,   COL8_848484);
    boxfill(buf, xs, 2,     ys-3,  60,     ys-2,   COL8_000000);
    boxfill(buf, xs, 60,    ys-24, 61,     ys-2,   COL8_000000);

    boxfill(buf, xs, xs-47, ys-24, xs-3,   ys-23,  COL8_848484);
    boxfill(buf, xs, xs-47, ys-23, xs-46,  ys-3,   COL8_848484);
    boxfill(buf, xs, xs-47, ys-3,  xs-3,   ys-2,   COL8_FFFFFF);
    boxfill(buf, xs, xs-3,  ys-24, xs-2,   ys-2,   COL8_FFFFFF);
}

void init_cursor(unsigned char *mouse) {
    static char cursor[16][16] = {
        ".*************..",
        "**OOOOOOOOOO*...",
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

void putstr8(unsigned char *buf, int xs, int bx0, int by0, unsigned char c, char *s) {
    unsigned char *row;
    char data;
    while (*s) {
        for(int i = 0; i < 16; i++) {
            row = buf + (by0 + i) * xs + bx0;
		    data = (FONT+(*s)*16)[i];
            for(int j = 0; j < 8; j++) {
                if(data & (0x80 >> j)) row[j] = c;
            }
        }
        bx0 += 8;
        s++;
    }
}
