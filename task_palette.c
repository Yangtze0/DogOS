
#include "dogos.h"

void Task_palette(void) {
    // 绘制图层
    unsigned char *sht_buf = (unsigned char *)malloc_4k(272 * 196);
    struct SHEET *sht = sheet_alloc((SHEETS.vxs-272)/2, (SHEETS.vys-196)/2, 272, 196, sht_buf, TASKS.now);
    sheet_make_window(sht, "palette");
    for(int i = 0; i < 16; i++) boxfill(sht->buf, sht->xs, 8+16*i, 28, 8+16*(i+1), 28+16*10, i);
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