
#include "dogos.h"

#define PIC0_ICW1   0x0020
#define PIC0_OCW2   0x0020
#define PIC0_IMR    0x0021
#define PIC0_ICW2   0x0021
#define PIC0_ICW3   0x0021
#define PIC0_ICW4   0x0021
#define PIC1_ICW1   0x00a0
#define PIC1_OCW2   0x00a0
#define PIC1_IMR    0x00a1
#define PIC1_ICW2   0x00a1
#define PIC1_ICW3   0x00a1
#define PIC1_ICW4   0x00a1

void init_pic(void) {
    io_out8(PIC0_IMR,   0xff);  // PCI0禁止所有中断
    io_out8(PIC1_IMR,   0xff);  // PCI1禁止所有中断

    io_out8(PIC0_ICW1,  0x11);  // 边沿触发模式
    io_out8(PIC0_ICW2,  0x20);  // IRQ0~7对应int20~27
    io_out8(PIC0_ICW3,  0x04);  // PCI1对应IRQ2
    io_out8(PIC0_ICW4,  0x01);  // 无缓冲区模式

    io_out8(PIC1_ICW1,  0x11);  // 边沿触发模式
    io_out8(PIC1_ICW2,  0x28);  // IRQ8~15对应int28~2f
    io_out8(PIC1_ICW3,  0x02);  // PCI1对应IRQ2
    io_out8(PIC1_ICW4,  0x01);  // 无缓冲区模式

    io_out8(PIC0_IMR,   0xfb);  // PCI1以外全部禁止
    io_out8(PIC1_IMR,   0xff);  // 禁止所有中断

    io_out8(PIC0_IMR,   0xf9);  // 开启int21 键盘中断
	io_out8(PIC1_IMR,   0xef);  // 开启int2c 鼠标中断
}

void inthandler21(int *esp) {
    putfonts8_asc(0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");

	for (;;) {
		io_hlt();
	}
}

void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67);
}

void inthandler2c(int *esp) {
    putfonts8_asc(0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");

	for (;;) {
		io_hlt();
	}
}

