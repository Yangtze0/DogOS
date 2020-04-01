
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
}

void init_idt(void) {
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADDR_IDT;

    for (int i = 0; i < (LIMIT_IDT+1)/8; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }

    set_gatedesc(idt + 0x20, (int) asm_inthandler20, 0x0008, AR_INTGATE32);
    set_gatedesc(idt + 0x21, (int) asm_inthandler21, 0x0008, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 0x0008, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 0x0008, AR_INTGATE32);
    set_gatedesc(idt + 0x30, (int) asm_dogos_api, 0x0008, AR_INTGATE32);
    load_idtr(LIMIT_IDT, (int)idt);
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
    gd->offset_low   = offset & 0xffff;
    gd->selector     = selector;
    gd->dw_count     = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high  = (offset >> 16) & 0xffff;
}

// inthandler20: timer.c

// inthandler21: keyboard.c

void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67);
}

// inthandler2c: mousu.c
