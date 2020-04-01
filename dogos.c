////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

void DogOS_main(void) {
    /* 获取启动配置信息 */
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;

    /* 初始化中断 */
    init_pic();                 // 初始化中断控制器
    init_idt();                 // 初始化中断描述符表
    init_pit(&TIMERS);          // 初始化定时器
    init_keyboard(&KEYBOARD);   // 初始化键盘及其缓冲区
    init_mouse(&MOUSE);         // 初始化鼠标及其缓冲区

    /* 初始化内存 */
    init_memory(&MEMORY);       // 0x00200000以上地址可分配，支持8MB内存管理

    /* 初始化多任务管理 */
    init_multitask(&TASKS);     // 时间片长度:10ms

    /* 初始化图形界面 */
    init_palette();             // 初始化调色板
    init_shtctl(&SHEETS, binfo);

    /* 系统启动就绪 */
    io_out8(PIC0_IMR, 0xf8);    // 开放int20(定时器)、int21(键盘)
	io_out8(PIC1_IMR, 0xef);    // 开放int2c(鼠标)
    io_sti();                   // 开放中断


    // 启动终端
    task_start((unsigned long)&Task_console);
    

    // 新建输入窗口
    unsigned char *buf_win = (unsigned char *)malloc_4k(160 * 52);
    struct SHEET *sht_win = sheet_alloc(80, 72, 160, 52, buf_win, TASKS.main);
    sheet_make_window(sht_win, "window");
    sheet_make_textbox(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
    sheet_updown(sht_win, SHEETS.top);
    int cursor_x = 8, cursor_c = COL8_FFFFFF;

    // 设置定时器
    struct FIFO8 timerfifo;
    char timerbuf[8];
    fifo8_init(&timerfifo, 8, timerbuf, TASKS.main);
    struct TIMER *timer_cursor = timer_alloc();
    timer_init(timer_cursor, &timerfifo, 1);
    timer_settime(timer_cursor, 50);

    for(;;) {
        // 主任务休眠，可被键盘鼠标缓冲区写入唤醒
        task_sleep(TASKS.main);

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

        // 鼠标中断响应
        while(fifo8_status(&MOUSE.fifo)) {
            io_cli();
            char byte_mouse = fifo8_get(&MOUSE.fifo);
            io_sti();
            if(mouse_decode(&MOUSE.mdec, byte_mouse)) {
                int mdx = MOUSE.mdec.x, mdy = MOUSE.mdec.y;
                sheet_slide(SHEETS.h[SHEETS.top], mdx, mdy);

                // 按下左键置顶、拖动窗口
                if(MOUSE.mdec.btn & 0x01) {
                    byte_mouse = SHEETS.vmap[SHEETS.h[SHEETS.top]->vy0*SHEETS.vxs+SHEETS.h[SHEETS.top]->vx0];
                    if(byte_mouse) {
                        if(byte_mouse != SHEETS.top-1) {
                            sheet_updown(SHEETS.h[byte_mouse], SHEETS.top-1);
                            KEYBOARD.fifo.task = SHEETS.h[SHEETS.top-1]->task;
                        }
                        if(SHEETS.h[SHEETS.top]->vx0 == 0 || SHEETS.h[SHEETS.top]->vx0 == SHEETS.vxs-1)
                            mdx = 0;
                        if(SHEETS.h[SHEETS.top]->vy0 == 0 || SHEETS.h[SHEETS.top]->vy0 == SHEETS.vys-1)
                            mdy = 0;
                        sheet_slide(SHEETS.h[byte_mouse], mdx, mdy);
                    }
                }
            }
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
