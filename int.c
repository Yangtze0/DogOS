
#include "dogos.h"

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

// inthandler21: keyboard.c

void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67);
}

// inthandler2c: mousu.c
