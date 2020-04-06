
# include "dogos.h"

struct CONSOLE cons;

void Task_console(void) {
    // 绘制图层
    cons.sht = sheet_alloc(80, 72, 256, 165, (unsigned char *)malloc_4k(256 * 165), TASKS.now);
    sheet_make_window(cons.sht, "console");
    boxfill(cons.sht->buf, cons.sht->xs, cons.sht->xs-21, 5, cons.sht->xs-5, 19, COL_WIN);
    sheet_make_textbox(cons.sht, 8, 28, 240, 128, COL8_000000);
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = COL8_000000;
    cons.fontc = COL8_00FF00;
    sheet_updown(cons.sht, SHEETS.top);
    
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
    char s[40], cmdline[32], i = 0;
    cons_putchar('>', 1);

    for (;;) {
        count++;
        task_sleep(TASKS.now);

        // 键盘中断响应
        while((cons.sht->height == SHEETS.top-1) && fifo8_status(&KEYBOARD.fifo)) {
            io_cli();
            unsigned char byte_ascii = fifo8_get(&KEYBOARD.fifo);
            io_sti();

            // 字符处理
            switch(byte_ascii) {
            case 0x08:  // 退格
                if(i) {
                    cons_putchar(' ', -1);
                    i--;
                }
                cmdline[i] = 0;
                break;
            case 0x0a:  // 回车
                cons_putchar(' ', 0);
                cmdline[i] = 0;
                i = 0;
                cons_runcmd(cmdline);
                cons_newline();
                cons_putchar('>', 1);
                break;
            default:    // 可显示字符
                if(i <= 30) {
                    cons_putchar(byte_ascii, 1);
                    cmdline[i++] = byte_ascii;
                }
                break;
            }

            // 光标再显示
            boxfill(cons.sht->buf, cons.sht->xs, cons.cur_x, cons.cur_y, cons.cur_x+8, cons.cur_y+16, cons.cur_c);
            sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x+8, cons.cur_y+16);
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
                sheet_putstr(cons.sht, 8, 140, s, strlen(s), COL8_0000FF, COL8_FFFFFF);
                break;
            case 1:
            case 0:
                if (data) {
                    timer->data = 0;
                    cons.cur_c = COL8_000000;
                } else {
                    timer->data = 1;
                    cons.cur_c = COL8_FFFFFF;
                }
                timer_settime(timer, 50);
                if(cons.sht->height == SHEETS.top-1) {
                    boxfill(cons.sht->buf, cons.sht->xs, cons.cur_x, cons.cur_y, cons.cur_x+8, cons.cur_y+16, cons.cur_c);
                    sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x+8, cons.cur_y+16);
                }
                break;
            default:
                break;
            }
        }
    }
}

void cons_putchar(int chr, char move) {
    char s[2];
    s[0] = chr;
    s[1] = 0;
    sheet_putstr(cons.sht, cons.cur_x, cons.cur_y, s, 1, COL8_000000, cons.fontc);

    if(move > 0) {
        cons.cur_x += 8;
        if(cons.cur_x == 248) cons_newline();
    } else if(move < 0) {
        cons.cur_x -= 8;
        if(cons.cur_x == 0) {
            cons.cur_y -= 16;
            cons.cur_x = 240;
        }
    } else {
        cons_newline();
    }
}

void cons_putstr(char *s) {
    while(*s) {
        cons_putchar(*s, 1);
        s++;
    }
}

void cons_newline(void) {
    int x, y;
    if (cons.cur_y < 124) {
        cons.cur_y += 16;
    } else {    // 末行滚屏
        for (y = 28; y < 124; y++) {
            for (x = 8; x < 248; x++) {
                cons.sht->buf[y*cons.sht->xs + x] = cons.sht->buf[(y+16)*cons.sht->xs + x];
            }
        }
		for(y = 124; y < 140; y++) {
            for(x = 8; x < 248; x++) {
                cons.sht->buf[y*cons.sht->xs + x] = COL8_000000;
            }
        }
        sheet_refresh(cons.sht, 8, 28, 240, 112);
    }
    cons.cur_x = 8;
}

void cons_runcmd(char *cmdline) {
    cons.fontc = COL8_FFFFFF;
    if(!strcmp(cmdline, "h")) {             // 帮助信息
        cmd_help();
    } else if(!strcmp(cmdline, "cls")) {    // 清屏
        cmd_cls();
    } else if(!strcmp(cmdline, "mem")) {    // 内存信息
        cmd_mem();
    } else if(!strcmp(cmdline, "color")) {  // 调色板程序
        task_start((unsigned long)&Task_palette);
    } else if(!strcmp(cmdline, "bball")) {  // 画图程序
        task_start((unsigned long)&Task_bball);
    } else if(!strcmp(cmdline, "invader")) {// 侵略者游戏
        task_start((unsigned long)&Task_invader);
    } else if(cmdline[0]) {                 // 错误指令
        cons.fontc = COL8_FF0000;
        cons_putstr("Bad command. \"h\" for help.");
    }
    cons.fontc = COL8_00FF00;
}

void cmd_help(void) {
    cons_putstr("The supported commands:");
    cons_newline();
    cons_putstr("    cls - clean the console.");
    cons_newline();
    cons_putstr("    mem - show memory info.");
    cons_newline();
    cons_putstr("    color - show palette.");
    cons_newline();
    cons_putstr("    bball - a beautiful ball.");
    cons_newline();
    cons_putstr("    invader - a famous game.");
}

void cmd_cls(void) {
    for (int y = 28; y < 140; y++) {
        for (int x = 8; x < 248; x++) {
            cons.sht->buf[y*cons.sht->xs + x] = COL8_000000;
        }
    }

    sheet_refresh(cons.sht, 8, 28, 240, 112);
    cons.cur_x = 8;
    cons.cur_y = 12;
}

void cmd_mem(void) {
    char s[40];
    sprintf(s, "Total:%04dMB, Free:%dKB", MEMORY.total, MEMORY.free*4);
    cons_putstr(s);
}
