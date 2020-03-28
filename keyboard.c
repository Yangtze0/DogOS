
#include "dogos.h"

struct KEYBOARDCTL KEYBOARD;

static char keytable0[0x80] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,   0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   0,   0,   ' ', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static char keytable1[0x80] = {
    0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,   'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0,   0,   0,   ' ', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);   // 通知PIC0已经受理IRQ-01

    unsigned char data = io_in8(PORT_KEYDAT), d2c = 0;
    switch(data) {
    case 0x0e:                  // 退格
        fifo8_put(&KEYBOARD.fifo, 0x08);
        break;
    case 0x2a:
        KEYBOARD.shift |= 1;    // 左shift ON
        break;
    case 0x36:
        KEYBOARD.shift |= 2;    // 右shift ON
        break;
    case 0x3a:
        KEYBOARD.capslock ^= 1; // capslock
        break;
    case 0xaa:
        KEYBOARD.shift &= ~1;   // 左shift OFF
        break;
    case 0xb6:
        KEYBOARD.shift &= ~2;   // 右shift OFF
        break;
    default:
        if(data < 0x80) {       // 可显示字符
            if(KEYBOARD.shift) {
                if(KEYBOARD.keytable1[data]) d2c = KEYBOARD.keytable1[data];
            } else {
                if(KEYBOARD.keytable0[data]) d2c = KEYBOARD.keytable0[data];
            }

            if(KEYBOARD.capslock) {
                if('a' <= d2c && d2c <= 'z') {
                    d2c -= 0x20;
                } else if('A' <= d2c && d2c <= 'Z') {
                    d2c += 0x20;
                }
            }

            if(d2c) fifo8_put(&KEYBOARD.fifo, d2c);
        }
        break;
    }
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
    kb->shift = 0;
    kb->capslock = 0;
    memcpy(kb->keytable0, keytable0, sizeof(keytable0));
    memcpy(kb->keytable1, keytable1, sizeof(keytable1));
}

