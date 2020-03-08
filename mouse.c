
#include "dogos.h"

struct FIFO8 MOUSE;

void inthandler2c(int *esp) {
    io_out8(PIC1_OCW2, 0x64);   // 通知PIC1已经受理IRQ-12
    io_out8(PIC0_OCW2, 0x62);   // 通知PIC0已经受理IRQ-02
    fifo8_put(&MOUSE, io_in8(PORT_KEYDAT));
}

void enable_mouse(struct MOUSE_DEC *mdec) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    mdec->phase = 0;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat) {
    switch (mdec->phase)
    {
    case 0:
        if(dat == 0xfa) mdec->phase = 1;
        return 0;
    case 1:
        if((dat & 0xc8) == 0x08) {          // 防止异常数据
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    case 2:
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    case 3:
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if(mdec->buf[0] & 0x10) mdec->x |= 0xffffff00;
        if(mdec->buf[0] & 0x20) mdec->y |= 0xffffff00;
        mdec->y = - mdec->y;                // 鼠标y向与画面符号相反
        return 1;
    }
    return -1;
}
