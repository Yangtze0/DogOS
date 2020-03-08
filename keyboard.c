
#include "dogos.h"

struct FIFO8 KEYBOARD;

void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);   // 通知PIC0已经受理IRQ-01
    fifo8_put(&KEYBOARD, io_in8(PORT_KEYDAT));
}

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

