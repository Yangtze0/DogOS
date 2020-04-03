
#include "dogos.h"

void Task_win(void) {
    // 绘制图层
    unsigned char *buf_win = (unsigned char *)malloc_4k(160 * 52);
    struct SHEET *sht_win = sheet_alloc((SHEETS.vxs-160)/2, (SHEETS.vys-52)/2, 160, 52, buf_win, TASKS.now);
    sheet_make_window(sht_win, "window");
    sheet_make_textbox(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
    sheet_updown(sht_win, SHEETS.top);
    int cursor_x = 8, cursor_c = COL8_FFFFFF;

    // 设置定时器
    struct FIFO8 timerfifo;
    char timerbuf[8];
    fifo8_init(&timerfifo, 8, timerbuf, TASKS.now);
    struct TIMER *timer_cursor = timer_alloc();
    timer_init(timer_cursor, &timerfifo, 1);
    timer_settime(timer_cursor, 50);

    for (;;) {
        task_sleep(TASKS.now);

        // 键盘中断响应
        while((sht_win->height == SHEETS.top-1) && fifo8_status(&KEYBOARD.fifo)) {
            io_cli();
            unsigned char byte_ascii = fifo8_get(&KEYBOARD.fifo);
            io_sti();

            // 字符处理
            char dbyte[2];
            memset(dbyte, 0 ,2);
            switch(byte_ascii) {
            case 0x08:  // 退格
                if(cursor_x > 8) {
                    sheet_putstr(sht_win, cursor_x, 28, " ", 1, COL8_FFFFFF, COL8_000000);
                    cursor_x -= 8;
                }
                break;
            case 0x0a:  // 回车
                break;
            default:    // 可显示字符
                if(cursor_x < 144) {
                    dbyte[0] = byte_ascii;
                    sheet_putstr(sht_win, cursor_x, 28, dbyte, 1, COL8_FFFFFF, COL8_000000);
                    cursor_x += 8;
                }
                break;
            }

            // 光标再显示
            boxfill(sht_win->buf, sht_win->xs, cursor_x, 28, cursor_x+8, 44, cursor_c);
            sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
        }

        // 定时器超时响应
        while(fifo8_status(&timerfifo)) {
            io_cli();
            char byte_timer = fifo8_get(&timerfifo);
            io_sti();
            switch (byte_timer) {
            case 1:
            case 0:
                if (byte_timer) {
                    timer_cursor->data = 0;
                    cursor_c = COL8_000000;
                } else {
                    timer_cursor->data = 1;
                    cursor_c = COL8_FFFFFF;
                }
                timer_settime(timer_cursor, 50);
                if(sht_win->height == SHEETS.top-1) {
                    boxfill(sht_win->buf, sht_win->xs, cursor_x, 28, cursor_x+8, 44, cursor_c);
                    sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
                }
                break;
            default:
                break;
            }
        }
    }
}