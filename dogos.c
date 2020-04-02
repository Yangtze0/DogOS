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

    for(;;) {
        // 主任务休眠，可被鼠标缓冲区写入唤醒
        task_sleep(TASKS.main);

        // 鼠标中断响应
        while(fifo8_status(&MOUSE.fifo)) {
            io_cli();
            char byte_mouse = fifo8_get(&MOUSE.fifo);
            io_sti();

            if(mouse_decode(&MOUSE.mdec, byte_mouse)) {
                // 鼠标坐标变化
                int mdx = MOUSE.mdec.x, mdy = MOUSE.mdec.y;
                // 鼠标图层
                struct SHEET *sht_m = SHEETS.h[SHEETS.top];
                // 指向图层高度
                char wh = SHEETS.vmap[sht_m->vy0 * SHEETS.vxs + sht_m->vx0];
                // 指向图层
                struct SHEET *sht_w = SHEETS.h[wh];

                // 移动鼠标图层
                sheet_slide(sht_m, mdx, mdy);

                // 按下左键
                if((MOUSE.mdec.btn & 0x01) && (wh > 0)){
                    // 置顶窗口
                    if(wh != SHEETS.top-1) sheet_updown(sht_w, SHEETS.top);

                    // 拖动窗口
                    if(sht_m->vx0 == 0 || sht_m->vx0 == SHEETS.vxs-1) mdx = 0;
                    if(sht_m->vy0 == 0 || sht_m->vy0 == SHEETS.vys-1) mdy = 0;
                    sheet_slide(sht_w, mdx, mdy);

                    // 关闭窗口(终端窗口无效)
                    if( (sht_w->task->next != TASKS.idle) && \
                        (sht_m->vx0 >= sht_w->vx0 + sht_w->xs - 21) && \
                        (sht_m->vx0 < sht_w->vx0 + sht_w->xs - 5) && \
                        (sht_m->vy0 >= sht_w->vy0 + 5) && \
                        (sht_m->vy0 < sht_w->vy0 + 19) ) {
                        task_end(sht_w->task);
                        sheet_updown(sht_w, 0);
                    }
                }
            }
        }
    }
}
