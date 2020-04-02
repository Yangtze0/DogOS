
/* nasm_func.asm */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_idtr(int limit, int addr);
void load_tr(int tr);
int load_cr0(void);
void store_cr0(int cr0);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);

void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);


/* boot.asm */
#define SEL_CODE        0x08
#define SEL_DATA        0x10
#define ADDR_BOOTINFO   0x00007e00

struct BOOTINFO {
    short reserve;
    char leds;
    char vmode;
    short scrnx, scrny;
    unsigned char *vram;
};


/* graphic.c */
#define COL8_000000     0   // 黑
#define COL8_FF0000     1   // 亮红
#define COL8_00FF00     2   // 亮绿
#define COL8_FFFF00     3   // 亮黄
#define COL8_0000FF     4   // 亮蓝
#define COL8_FF00FF     5   // 亮紫
#define COL8_00FFFF     6   // 浅亮蓝
#define COL8_FFFFFF     7   // 白
#define COL8_C6C6C6     8   // 亮灰
#define COL8_840000     9   // 暗红
#define COL8_008400     10  // 暗绿
#define COL8_848400     11  // 暗黄
#define COL8_000084     12  // 暗青
#define COL8_840084     13  // 暗紫
#define COL8_008484     14  // 浅暗蓝
#define COL8_848484     15  // 暗灰

#define COL_INVISIBLE   99  // 透明
#define COL_WINFONT     97  // 窗口标题字体色
#define COL_WIN         98  // 窗口标题背景色

#define MOUSEX          16
#define MOUSEY          16

void init_palette(void);
void boxfill(unsigned char *buf, int bxs, int bx0, int by0, int bx1, int by1, unsigned char c);
void init_screen(unsigned char *buf, int xs, int ys);
void init_cursor(unsigned char *buf_mouse);
void putstr8(unsigned char *buf, int xs, int bx0, int by0, unsigned char c, char *s);


/* string.c */
void * memset(void *s, int c, unsigned long count);
void * memcpy(void *d, const void *s, unsigned long count);
unsigned long strlen(const char *s);
char *strcpy(char * dest, const char *src);
int strcmp(const char *src, const char *dst);
int sprintf(char *s, const char *fmt, ...);


/* interrupt.c */
#define PIC0_ICW1   0x0020
#define PIC0_OCW2   0x0020
#define PIC0_IMR    0x0021
#define PIC0_ICW2   0x0021
#define PIC0_ICW3   0x0021
#define PIC0_ICW4   0x0021
#define PIC1_ICW1   0x00a0
#define PIC1_OCW2   0x00a0
#define PIC1_IMR    0x00a1
#define PIC1_ICW2   0x00a1
#define PIC1_ICW3   0x00a1
#define PIC1_ICW4   0x00a1

#define ADDR_IDT        0x00020000
#define LIMIT_IDT       0x000007ff
#define AR_INTGATE32    0x008e

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_idt(void);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void init_pic(void);
void inthandler27(int *esp);


/* fifo.c */
#define FLAGS_OVERRUN   0x0001

struct FIFO8 {
    char *buf;
    int r, w, size, free, flags;
    struct TASK *task;
};

void fifo8_init(struct FIFO8 *fifo, int size, char *buf, struct TASK *task);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);


/* keyboard.c */
#define PORT_KEYDAT             0x0060
#define PORT_KEYSTA             0x0064
#define PORT_KEYCMD             0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE       0x60
#define KBC_MODE                0x47

struct KEYBOARDCTL {
    char buffer[32];
    struct FIFO8 fifo;
    unsigned char shift, capslock;
    char keytable0[0x80];
    char keytable1[0x80];
};

void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct KEYBOARDCTL *kb);


/* mouse.c */
#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

struct MOUSECTL {
    char cursor[MOUSEX*MOUSEY];
    char buffer[128];
    struct FIFO8 fifo;
    struct MOUSE_DEC mdec;
};

void inthandler2c(int *esp);
void init_mouse(struct MOUSECTL *mouse);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


/* memory.c */
#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

struct MEMORYCTL {
    unsigned char bitmap[256];  // 8MB
    int total, free;            // 物理内存(MB), 空闲内存(4KB)
};

void init_memory(struct MEMORYCTL *m);
int mtest(unsigned long start, unsigned long end);
unsigned long malloc_4k(unsigned int size);
int mfree_4k(unsigned long addr, unsigned int size);


/* timer.c */
#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040
#define MAX_TIMER   100
#define TIMER_FLAGS_ALLOC   1   // 定时器已配置
#define TIMER_FLAGS_USING   2   // 定时器已运行

struct TIMER {
    struct TIMER *next;
    unsigned int timeout, flags;
    struct FIFO8 *fifo;
    unsigned char data;
};

struct TIMERCTL {
    unsigned int time;
    struct TIMER *t0;
    struct TIMER timer[MAX_TIMER];
};

void init_pit(struct TIMERCTL *timers);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);


/* mutitask.c */
#define ADDR_GDT        0x00010000
#define AR_DATA32_RW    0x4092
#define AR_CODE32_ER    0x409a
#define AR_TSS32        0x0089
#define MAX_TASKS       100
#define TASK_GDT0       3

#define TASK_UNUSE      0
#define TASK_SLEEP      1
#define TASK_RUNNING    2

struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct TSS32 {
    unsigned long backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    unsigned long eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    unsigned long es, cs, ss, ds, fs, gs;
    unsigned long ldtr, iomap;
};

struct TASK {
    struct TASK *next;
    int sel, flags;
    struct TSS32 tss;
};

struct TASKCTL {
    struct TASK *now;
    struct TASK *main;
    struct TASK *idle;
    struct TASK tasks[MAX_TASKS];
};

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void init_multitask(struct TASKCTL *mt);
struct TASK *task_alloc(void);
void task_run(struct TASK *task);
void task_end(struct TASK *task);
void task_switch(void);
void task_sleep(struct TASK *task);
void task_start(unsigned long task_entry);


/* sheet.c */
#define MAX_SHEETS      256

struct SHEET {
    int vx0, vy0, xs, ys;
    unsigned char *buf, height, flags;  // 图层内容、高度、标志
    struct TASK *task;
};

struct SHEETCTL {
    unsigned char *vram, *vmap;
    int vxs, vys, top;
    struct SHEET *h[MAX_SHEETS];
    struct SHEET sheet[MAX_SHEETS];
};

void init_shtctl(struct SHEETCTL *ss, struct BOOTINFO *binfo);
struct SHEET *sheet_alloc(int vx0, int vy0, int bxs, int bys, unsigned char *buf, struct TASK *task);
void sheet_updown(struct SHEET *sht, unsigned char height);
void sheet_refreshmap(int h0, int vx0, int vy0, int xs, int ys);
void sheet_refreshsub(int h0, int h1, int vx0, int vy0, int xs, int ys);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bxs, int bys);
void sheet_slide(struct SHEET *sht, int dx, int dy);
void sheet_free(struct SHEET *sht);
void sheet_make_window(struct SHEET *sht, char *title);
void sheet_make_textbox(struct SHEET *sht, int bx0, int by0, int bxs, int bys, int c);
void sheet_putstr(struct SHEET *sht, int bx0, int by0, char *s, int l, int b, int c);


/* console.c */
struct CONSOLE {
    struct SHEET *sht;
    int cur_x, cur_y, cur_c, fontc;
};

void Task_console(void);
void cons_putchar(int chr, char move);
void cons_putstr(char *s);
void cons_newline(void);
void cons_runcmd(char *cmdline);
void cmd_help(void);
void cmd_cls(void);
void cmd_mem(void);

// 应用程序
void Task_win(void);


/* dogos.c */
extern struct KEYBOARDCTL   KEYBOARD;
extern struct MOUSECTL      MOUSE;
extern struct MEMORYCTL     MEMORY;
extern struct SHEETCTL      SHEETS;
extern struct TIMERCTL      TIMERS;
extern struct TASKCTL       TASKS;
