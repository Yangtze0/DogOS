
#include "dogos.h"

struct POINT {
    int x, y;
};

static struct POINT table[16] = {
    { 204, 129 }, { 195,  90 }, { 172,  58 }, { 137,  38 }, {  98,  34 },
    {  61,  46 }, {  31,  73 }, {  15, 110 }, {  15, 148 }, {  31, 185 },
    {  61, 212 }, {  98, 224 }, { 137, 220 }, { 172, 200 }, { 195, 168 },
    { 204, 129 }
};

void Task_bball(void) {
    // 绘制图层
    unsigned char *sht_buf = (unsigned char *)malloc_4k(216 * 237);
    struct SHEET *sht = sheet_alloc((SHEETS.vxs-216)/2, (SHEETS.vys-237)/2, 216, 237, sht_buf, TASKS.now);
    sheet_make_window(sht, "bball");
    boxfill(sht->buf, sht->xs, 8, 29, 207, 228, 0);

    int i, j, dis;
    for (i = 0; i <= 14; i++) {
        for (j = i + 1; j <= 15; j++) {
            dis = j - i;
            if (dis >= 8) {
                dis = 15 - dis;
            }
            if (dis != 0) {
                drawline(sht->buf, sht->xs, table[i].x, table[i].y, table[j].x, table[j].y, 8 - dis);
            }
        }
    }

    sheet_updown(sht, SHEETS.top);

    for (;;) {
        task_sleep(TASKS.now);

        // 键盘中断响应
        while((sht->height == SHEETS.top-1) && fifo8_status(&KEYBOARD.fifo)) {
            io_cli();
            unsigned char byte_ascii = fifo8_get(&KEYBOARD.fifo);
            io_sti();
        }
    }
}