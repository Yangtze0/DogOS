
/* nasm_func.asm */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);


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

void init_palette(void);
void boxfill8(int x0, int y0, int x1, int y1, unsigned char color);
void init_screen(void);
void putfont8(int x, int y, unsigned char color, char *font);
void putfonts8_asc(int x, int y, unsigned char color, char *s);
void init_mouse_cursor8(char *mouse);
void putblock8_8(int x0, int y0, int sx, int sy, char *mouse);


/* dsctbl.c */
struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_idt(void);
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
};

void fifo8_init(struct FIFO8 *fifo, int size, char *buf);
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

