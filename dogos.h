
/* nasm_func.asm */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
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
void init_mouse_cursor8(char *mouse, unsigned char back_color);
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
void init_pic(void);
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);

