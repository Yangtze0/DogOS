
#include "dogos.h"

void fifo8_init(struct FIFO8 *fifo, int size, char *buf, struct TASK *task) {
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->r = 0;
    fifo->w = 0;
    fifo->task = task;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data) {
    if (fifo->free == 0) {
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
	}
    fifo->buf[fifo->r] = data;
    fifo->r++;
    fifo->r %= fifo->size;
    fifo->free--;

    // 任务唤醒
    if(fifo->task && (fifo->task->flags == TASK_SLEEP)) fifo->task->flags = TASK_RUNNING;
    return 0;
}

int fifo8_get(struct FIFO8 *fifo) {
    int data;
    if (fifo->free == fifo->size) return -1;
    data = fifo->buf[fifo->w];
    fifo->w++;
    fifo->w %= fifo->size;
    fifo->free++;
    return data;
}

int fifo8_status(struct FIFO8 *fifo) {
    return fifo->size - fifo->free;
}
