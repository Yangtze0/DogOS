
#include "dogos.h"

extern struct TIMER *mt_timer;
struct TIMERCTL TIMERS;

void init_pit(void) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);    // 中断频率:100Hz

    TIMERS.count = 0;
    TIMERS.next = 0xffffffff;
    for(int i = 0; i < MAX_TIMER; i++) {
        TIMERS.timer[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void) {
    for (int i = 0; i < MAX_TIMER; i++) {
        if (TIMERS.timer[i].flags == 0) {
            TIMERS.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &TIMERS.timer[i];
        }
    }

    return 0;
}

void timer_free(struct TIMER *timer) {
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data) {
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout) {
    timer->timeout = timeout + TIMERS.count;
    timer->flags = TIMER_FLAGS_USING;
    if(TIMERS.next > timer->timeout) TIMERS.next = timer->timeout;
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);
    TIMERS.count++;
    if(TIMERS.count < TIMERS.next) return;
    char ts = 0;

    for(int i = 0; i < MAX_TIMER; i++) {
        if(TIMERS.timer[i].flags == TIMER_FLAGS_USING) {
            if(TIMERS.timer[i].timeout <= TIMERS.count) {
                if(&TIMERS.timer[i] == mt_timer) {
                    ts = 1;
                    break;
                }
                TIMERS.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(TIMERS.timer[i].fifo, TIMERS.timer[i].data);
            } else if(TIMERS.timer[i].timeout < TIMERS.next) {
                TIMERS.next = TIMERS.timer[i].timeout;
            }
        }
    }

    if(ts) mt_taskswitch();
}
