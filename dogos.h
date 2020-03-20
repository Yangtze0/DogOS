
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
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);


/* boot.asm */
struct BOOTINFO {
    short reserve;
    char leds;
    char vmode;
    short scrnx, scrny;
    unsigned char *vram;
};
#define ADR_BOOTINFO    0x00007e00


/* graphic.c */
#define COL8_000000     0
#define COL8_FF0000     1
#define COL8_00FF00     2
#define COL8_FFFF00     3
#define COL8_0000FF     4
#define COL8_FF00FF     5
#define COL8_00FFFF     6
#define COL8_FFFFFF     7
#define COL8_C6C6C6     8
#define COL8_840000     9
#define COL8_008400     10
#define COL8_848400     11
#define COL8_000084     12
#define COL8_840084     13
#define COL8_008484     14
#define COL8_848484     15
#define COL_INVISIBLE   99

void init_palette(void);
void boxfill8(unsigned char *vram, int xs, int x0, int y0, int x1, int y1, unsigned char color);
void init_screen(unsigned char *vram, int xm, int ym);
void putfont8(unsigned char *vram, int xs, int x, int y, unsigned char color, char *font);
void putfonts8_asc(unsigned char *vram, int xs, int x, int y, unsigned char color, char *s);
void init_mouse_cursor8(unsigned char *mouse);


/* dsctbl.c */
#define ADDR_IDT        0x00020000
#define LIMIT_IDT       0x000007ff
#define ADR_GDT         0x00010000
#define LIMIT_GDT       0x0000ffff
#define AR_DATA32_RW    0x4092
#define AR_CODE32_ER    0x409a
#define AR_INTGATE32    0x008e
#define AR_TSS32        0x0089

struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_idt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


/* int.c */
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

void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);


/* mouse.c */
#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

void inthandler2c(int *esp);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


/* memory.c */
#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000
#define MEMMAN_FREES        4090

struct FREEINFO {
    unsigned int addr, size;
};

struct MEMMAN {
    int frees, maxfrees, lostsize, losts;
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(void);
unsigned int memman_alloc(unsigned int size);
int memman_free(unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(unsigned int size);
int memman_free_4k(unsigned int addr, unsigned int size);


/* sheet.c */
#define MAX_SHEETS      256

struct SHEET {
    unsigned char *buf, height;     // 图层内容、高度
    int x0, y0, xs, ys, flags;
};

struct SHEETCTL {
    unsigned char *vram, *vmap;
    int xs, ys, top;
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};

void shtctl_init(struct SHEETCTL *ss, unsigned char *vram, int xs, int ys);
struct SHEET *sheet_alloc(int x0, int y0, int xs, int ys, unsigned char *buf);
void sheet_updown(struct SHEET *sht, unsigned char height);
void sheet_refreshmap(int h0, int x0, int y0, int x1, int y1);
void sheet_refresh(struct SHEET *sht, int x0, int y0, int x1, int y1);
void sheet_refreshsub(int h0, int h1, int x0, int y0, int x1, int y1);
void sheet_slide(struct SHEET *sht, int x0, int y0);
void sheet_free(struct SHEET *sht);


/* timer.c */
#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040
#define MAX_TIMER   500
#define TIMER_FLAGS_ALLOC   1   // 定时器已配置
#define TIMER_FLAGS_USING   2   // 定时器已运行

struct TIMER {
    unsigned int timeout, flags;
    struct FIFO8 *fifo;
    unsigned char data;
};

struct TIMERCTL {
    unsigned int count, next;
    struct TIMER timer[MAX_TIMER];
};

void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);


/* mutitask.c */
#define MAX_TASKS       100
#define TASK_GDT0       3

struct TSS32 {
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};

struct TASK {
    int sel, flags;
    struct TSS32 tss;
};

struct TASKCTL {
    int running;        // 运行任务数量
    int now;            // 当前任务标识
    struct TASK *tasks[MAX_TASKS];
    struct TASK tasks0[MAX_TASKS];
};

struct TASK *task_init(void);
struct TASK *task_alloc(void);
void task_run(struct TASK *task);
void task_switch(void);
void task_sleep(struct TASK *task);
