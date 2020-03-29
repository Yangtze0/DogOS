
# include "dogos.h"

int cons_newline(int cursor_y, struct SHEET *sht) {
    int x, y;
    if (cursor_y < 124) {
        cursor_y += 16;
    } else {    // 末行滚屏
        for (y = 28; y < 124; y++) {
            for (x = 8; x < 248; x++) {
                sht->buf[y*sht->xs + x] = sht->buf[(y+16)*sht->xs + x];
            }
        }
		for(y = 124; y < 140; y++) {
            for(x = 8; x < 248; x++) {
                sht->buf[y*sht->xs + x] = COL8_000000;
            }
        }
        sheet_refresh(sht, 8, 28, 240, 112);
    }
    return cursor_y;
}

int cons_cls(struct SHEET *sht) {
    for (int y = 28; y < 140; y++) {
        for (int x = 8; x < 248; x++) {
            sht->buf[y*sht->xs + x] = COL8_000000;
        }
    }

    sheet_refresh(sht, 8, 28, 240, 112);
    return 12;
}

void Task_console(void) {
    // 绘制图层
    unsigned char *sht_buf = (unsigned char *)malloc_4k(256 * 165);
    struct SHEET *sht = sheet_alloc(80, 144, 256, 165, sht_buf, TASKS.now);
    sheet_make_window(sht, "console");
    sheet_make_textbox(sht, 8, 28, 240, 128, COL8_000000);
    sheet_updown(sht, SHEETS.top);
    
    // 设置定时器
    struct FIFO8 timerfifo;
    char timerbuf[8];
    fifo8_init(&timerfifo, 8, timerbuf, TASKS.now);
    struct TIMER *timer = timer_alloc();
    timer_init(timer, &timerfifo, 1);
    timer_settime(timer, 50);
    struct TIMER *speed = timer_alloc();
    timer_init(speed, &timerfifo, 2);
    timer_settime(speed, 100);

    // 状态变量
    unsigned int count = 0;
    char dbyte[2], s[40], cmdline[30];
    memset(dbyte, 0 ,2);
    int cursor_x = 16, cursor_y = 28, cursor_c = COL8_000000;
    sheet_putstr(sht, cursor_x-8, cursor_y, ">", 1, COL8_000000, COL8_00FF00);

    for (;;) {
        count++;
        task_sleep(TASKS.now);

        // 键盘中断响应
        while((sht->height == SHEETS.top-1) && fifo8_status(&KEYBOARD.fifo)) {
            io_cli();
            unsigned char byte_ascii = fifo8_get(&KEYBOARD.fifo);
            io_sti();

            // 字符处理
            switch(byte_ascii) {
            case 0x08:  // 退格
                if(cursor_x > 16) {
                    sheet_putstr(sht, cursor_x, cursor_y, " ", 1, COL8_000000, COL8_000000);
                    cursor_x -= 8;
                }
                break;
            case 0x0a:  // 回车
                sheet_putstr(sht, cursor_x, cursor_y, " ", 1, COL8_000000, COL8_000000);
                cmdline[cursor_x/8 - 2] = 0;
                cursor_x = 16;
                cursor_y = cons_newline(cursor_y, sht);

                // 执行命令
                if(!strcmp(cmdline, "h")) {             // 帮助信息
                    sheet_putstr(sht, 8, cursor_y, "The supported instructions:", 30, COL8_000000, COL8_FFFFFF);
                    cursor_y = cons_newline(cursor_y, sht);
                    sheet_putstr(sht, 8, cursor_y, "    mem - show memory info.", 30, COL8_000000, COL8_FFFFFF);
                    cursor_y = cons_newline(cursor_y, sht);
                    sheet_putstr(sht, 8, cursor_y, "    cls - clean the console.", 30, COL8_000000, COL8_FFFFFF);
                } else if(!strcmp(cmdline, "cls")) {    // 清屏
                    cursor_y = cons_cls(sht);
                } else if(!strcmp(cmdline, "mem")) {    // 内存信息
                    sprintf(s, "Total:%04dMB, Free:%dKB", MEMORY.total, MEMORY.free*4);
                    sheet_putstr(sht, 8, cursor_y, s, strlen(s), COL8_000000, COL8_FFFFFF);
                } else if(cmdline[0]) {                 // 错误指令
                    sheet_putstr(sht, 8, cursor_y, "Bad command. \"h\" for help.", 27, COL8_000000, COL8_FFFFFF);
                }
                cursor_y = cons_newline(cursor_y, sht);
                sheet_putstr(sht, cursor_x-8, cursor_y, ">", 1, COL8_000000, COL8_00FF00);
                break;
            default:    // 可显示字符
                if(cursor_x < 240) {
                    dbyte[0] = byte_ascii;
                    sheet_putstr(sht, cursor_x, cursor_y, dbyte, 1, COL8_000000, COL8_00FF00);
                    cmdline[cursor_x/8 - 2] = byte_ascii;
                    cursor_x += 8;
                }
                break;
            }

            // 光标再显示
            boxfill(sht->buf, sht->xs, cursor_x, cursor_y, cursor_x+8, cursor_y+16, cursor_c);
            sheet_refresh(sht, cursor_x, cursor_y, cursor_x+8, cursor_y+16);
        }

        // 定时器超时响应
        while(fifo8_status(&timerfifo)) {
            io_cli();
            unsigned char data = fifo8_get(&timerfifo);
            io_sti();
            switch (data) {
            case 2:
                timer_settime(speed, 100);
                sprintf(s, "Running speed:%08x loops/s", count);
                count = 0;
                sheet_putstr(sht, 8, 140, s, strlen(s), COL8_0000FF, COL8_FFFFFF);
                break;
            case 1:
            case 0:
                if (data) {
                    timer->data = 0;
                    cursor_c = COL8_000000;
                } else {
                    timer->data = 1;
                    cursor_c = COL8_FFFFFF;
                }
                timer_settime(timer, 50);
                if(sht->height == SHEETS.top-1) {
                    boxfill(sht->buf, sht->xs, cursor_x, cursor_y, cursor_x+8, cursor_y+16, cursor_c);
                    sheet_refresh(sht, cursor_x, cursor_y, cursor_x+8, cursor_y+16);
                }
                break;
            default:
                break;
            }
        }
    }
}
