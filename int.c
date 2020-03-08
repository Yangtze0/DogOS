
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

#define PORT_KEYDAT 0x0060

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

struct FIFO8 KEYBOARD;

void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);   // 通知PIC0已经受理IRQ-01
    fifo8_put(&KEYBOARD, io_in8(PORT_KEYDAT));
}

void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67);
}

struct FIFO8 MOUSE;

void inthandler2c(int *esp) {
    io_out8(PIC1_OCW2, 0x64);   // 通知PIC1已经受理IRQ-12
    io_out8(PIC0_OCW2, 0x62);   // 通知PIC0已经受理IRQ-02
    fifo8_put(&MOUSE, io_in8(PORT_KEYDAT));
}

