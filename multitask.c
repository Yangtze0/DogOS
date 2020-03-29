
#include "dogos.h"

struct TASKCTL TASKS;
struct TIMER *task_timer;

void Task_idle(void) {
    for(;;) {
        io_hlt();
    }
}

void init_multitask(struct TASKCTL *mt) {
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADDR_GDT;
    for (int i = 0; i < MAX_TASKS; i++) {
        mt->tasks[i].flags = TASK_UNUSE;
        mt->tasks[i].sel = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &mt->tasks[i].tss, AR_TSS32);
    }
    mt->main = task_alloc();
    mt->main->flags = TASK_RUNNING;
    mt->main->next = mt->main;
    mt->now = mt->main;
    task_start((unsigned long)&Task_idle);
    mt->idle = mt->main->next;

    load_tr(mt->main->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, 1);
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000;
        limit /= 0x1000;
    }
    sd->limit_low    = limit & 0xffff;
    sd->base_low     = base & 0xffff;
    sd->base_mid     = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high    = (base >> 24) & 0xff;
}

struct TASK *task_alloc(void) {
    struct TASK *task = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (TASKS.tasks[i].flags == TASK_UNUSE) {
            task = &TASKS.tasks[i];
        }
    }
    if(task == 0) return 0;     // 全部正在使用

    task->next = 0;
    task->flags = TASK_SLEEP;
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

void task_run(struct TASK *task) {
    task->flags = TASK_RUNNING;
    task->next = TASKS.main->next;
    TASKS.main->next = task;
}

void task_switch(void) {
    struct TASK *ts = TASKS.now;
    for(;;) {
        ts = ts->next;
        if(ts == TASKS.now) {
            if(ts->flags == TASK_SLEEP) {   // 所有task均在sleep
                TASKS.idle->flags = TASK_RUNNING;
                ts = TASKS.idle;
                break;
            }
            return;
        }
        if(ts->flags == TASK_RUNNING) break;
    }

    if(ts != TASKS.idle && TASKS.idle->flags == TASK_RUNNING) TASKS.idle->flags = TASK_SLEEP;
    TASKS.now = ts;
    farjmp(0, ts->sel);
}

void task_sleep(struct TASK *task) {
    if(task->flags == TASK_RUNNING) task->flags = TASK_SLEEP;
    if(task == TASKS.now) task_switch();
}

void task_start(unsigned long task_entry) {
    struct TASK *new_task = task_alloc();
    new_task->tss.esp = malloc_4k(4*1024) + 4*1024;
    new_task->tss.eip = task_entry;
    task_run(new_task);
}
