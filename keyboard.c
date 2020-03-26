
#include "dogos.h"

struct KEYBOARDCTL KEYBOARD;

static char keytable[0x54] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.'
};

void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);   // 通知PIC0已经受理IRQ-01
    fifo8_put(&KEYBOARD.fifo, io_in8(PORT_KEYDAT));
}

void wait_KBC_sendready(void) {
    while(1) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) break;
	}
}

void init_keyboard(struct KEYBOARDCTL *kb) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    fifo8_init(&kb->fifo, 32, kb->buffer, 0);
    memcpy(kb->keytable, keytable, sizeof(keytable));
}

