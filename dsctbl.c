
#include "dogos.h"

#define ADDR_IDT        0x00020000

#define AR_DATA32_RW    0x4092
#define AR_CODE32_ER    0x409a
#define AR_INTGATE32    0x008e

void init_idt(void) {
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADDR_IDT;

    for (int i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }

    set_gatedesc(idt + 0x21, (int) asm_inthandler21, 0x0008, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 0x0008, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 0x0008, AR_INTGATE32);
    load_idtr(0x7ff, (int)idt);
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
    gd->offset_low   = offset & 0xffff;
    gd->selector     = selector;
    gd->dw_count     = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high  = (offset >> 16) & 0xffff;
}
