
#include "dogos.h"

extern struct TIMER *task_timer;
struct TIMERCTL TIMERS;

void init_pit(struct TIMERCTL *timers) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);    // 中断频率:100Hz

    timers->time = 0;           // 单位:10ms
    for(int i = 0; i < MAX_TIMER; i++) {
        timers->timer[i].flags = 0;
    }

    // 设置哨兵
	timers->t0 = timer_alloc();
    timers->t0->timeout = 0xffffffff;
	timers->t0->flags = TIMER_FLAGS_USING;
	timers->t0->next = 0;
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);   // 通知PIC0已经受理IRQ-00
    TIMERS.time++;
    if(TIMERS.t0->timeout > TIMERS.time) return;

    struct TIMER *timer = TIMERS.t0;
    char ts = 0;
    for(;;) {
        if(timer->timeout > TIMERS.time) break;

        // 超时
        timer->flags = TIMER_FLAGS_ALLOC;
        if(timer == task_timer) {
            ts = 1;
        } else {
            fifo8_put(timer->fifo, timer->data);
        }
        timer = timer->next;
    }
    TIMERS.t0 = timer;

    if(ts) {
        timer_settime(task_timer, 1);
        task_switch();
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
    timer->timeout = timeout + TIMERS.time;
    timer->flags = TIMER_FLAGS_USING;

    int e = io_load_eflags();
    io_cli();
    struct TIMER *s, *t = TIMERS.t0;

    // 插入到最前面
    if (timer->timeout <= t->timeout) {
        TIMERS.t0 = timer;
        timer->next = t;
        io_store_eflags(e);
        return;
    }

    // 寻找插入位置
    for (;;) {
        s = t;
        t = s->next;
        if (timer->timeout <= t->timeout) {
            s->next = timer;
            timer->next = t;
            io_store_eflags(e);
            return;
        }
    }
}
