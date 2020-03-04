////////////////////////////////////////////////////////////////////////////////
//  DogOS
////////////////////////////////////////////////////////////////////////////////

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

//  VGA320*200*8
void init_palette(void);
void boxfill8(int x0, int y0, int x1, int y1, unsigned char color);

#define COL8_000000     0
#define COL8_FF0000     1
#define COL8_00FF00     2
#define COL8_FFFF00     3
#define COL8_0000FF     4
#define COL8_FF00FF     5
#define COL8_00FFFF     6
#define COL8_FFFFFF     7
#define COL8_C6C6C6     8
#define COL8_840000     9
#define COL8_008400     10
#define COL8_848400     11
#define COL8_000084     12
#define COL8_840084     13
#define COL8_008484     14
#define COL8_848484     15

void DogOS_main(void) {
    init_palette();

    int xm = 320;
    int ym = 200;
    boxfill8(0,     0,      xm-1,   ym-29,  COL8_008484);
    boxfill8(0,     ym-28,  xm-1,   ym-28,  COL8_C6C6C6);
	boxfill8(0,     ym-27,  xm-1,   ym-27,  COL8_FFFFFF);
    boxfill8(0,     ym-26,  xm-1,   ym-1,   COL8_C6C6C6);

	boxfill8(3,     ym-24,  59,     ym-24,  COL8_FFFFFF);
	boxfill8(2,     ym-24,  2,      ym-4,   COL8_FFFFFF);
	boxfill8(3,     ym-4,   59,     ym-4,   COL8_848484);
	boxfill8(59,    ym-23,  59,     ym-5,   COL8_848484);
	boxfill8(2,     ym-3,   59,     ym-3,   COL8_000000);
	boxfill8(60,    ym-24,  60,     ym-3,   COL8_000000);

	boxfill8(xm-47, ym-24,  xm-4,   ym-24,  COL8_848484);
	boxfill8(xm-47, ym-23,  xm-47,  ym-4,   COL8_848484);
	boxfill8(xm-47, ym-3,   xm-4,   ym-3,   COL8_FFFFFF);
	boxfill8(xm-3,  ym-24,  xm-3,   ym-3,   COL8_FFFFFF);

    for(;;) {
        io_hlt();
    }
}

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

void boxfill8(int x0, int y0, int x1, int y1, unsigned char color) {
    unsigned char *vram = (unsigned char *)0xa0000;
	for (int y = y0; y <= y1; y++) {
		for (int x = x0; x <= x1; x++)
			vram[y * 320 + x] = color;
	}
}
