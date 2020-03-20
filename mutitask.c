
#include "dogos.h"

struct TASKCTL TASKS;
struct TIMER *task_timer;

struct TASK * task_init(void) {
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    for (int i = 0; i < MAX_TASKS; i++) {
        TASKS.tasks0[i].flags = 0;
        TASKS.tasks0[i].sel = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &TASKS.tasks0[i].tss, AR_TSS32);
    }
    struct TASK *task = task_alloc();
    task->flags = 2;    // 任务活动标志
    TASKS.running = 1;
    TASKS.now = 0;
    TASKS.tasks[0] = task;
    load_tr(task->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, 2);
    return task;
}

struct TASK *task_alloc(void) {
    struct TASK *task;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (TASKS.tasks0[i].flags == 0) {
            task = &TASKS.tasks0[i];
            task->flags = 1;                // 任务使用标志
            task->tss.eflags = 0x00000202;  // IF = 1;
            task->tss.eax = 0;
            task->tss.ecx = 0;
            task->tss.edx = 0;
            task->tss.ebx = 0;
            task->tss.ebp = 0;
            task->tss.esi = 0;
            task->tss.edi = 0;
            task->tss.cs = 8*1;
            task->tss.ds = 8*2;
            task->tss.es = 8*2;
            task->tss.fs = 8*2;
            task->tss.gs = 8*2;
            task->tss.ss = 8*2;
            task->tss.ldtr = 0;
            task->tss.iomap = 0x40000000;
            return task;
        }
    }
    return 0;   // 全部正在使用
}

void task_run(struct TASK *task) {
    task->flags = 2;
    TASKS.tasks[TASKS.running] = task;
    TASKS.running++;
}

void task_switch(void) {
    timer_settime(task_timer, 2);
    if (TASKS.running >= 2) {
        TASKS.now++;
        if (TASKS.now == TASKS.running) TASKS.now = 0;
		farjmp(0, TASKS.tasks[TASKS.now]->sel);
    }
}

void task_sleep(struct TASK *task) {
    int i;
    char ts = 0;
    if (task->flags == 2) {     // 唤醒状态
        if (task == TASKS.tasks[TASKS.now]) ts = 1;

        // 寻找task所在位置
        for (i = 0; i < TASKS.running; i++) {
            if (TASKS.tasks[i] == task) break;
        }
        TASKS.running--;

        // 移动成员
        if (i < TASKS.now) TASKS.now--;
        for (; i < TASKS.running; i++) {
            TASKS.tasks[i] = TASKS.tasks[i + 1];
        }

        task->flags = 1;        // 不工作状态
        if (ts) {
            if (TASKS.now >= TASKS.running) TASKS.now = 0;
            farjmp(0, TASKS.tasks[TASKS.now]->sel);
        }
    }
}
