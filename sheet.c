
#include "dogos.h"

extern struct MOUSECTL MOUSE;
struct SHEETCTL SHEETS;

void init_shtctl(struct SHEETCTL *ss, struct BOOTINFO *binfo) {
    ss->vram = binfo->vram;
    ss->vxs = binfo->scrnx;
    ss->vys = binfo->scrny;
    ss->vmap = (unsigned char *)malloc_4k(ss->vxs * ss->vys);

    // 初始化背景图层
    ss->sheet[0].flags = 1;
    ss->sheet[0].vx0 = 0;
    ss->sheet[0].vy0 = 0;
    ss->sheet[0].xs = ss->vxs;
    ss->sheet[0].ys = ss->vys;
    ss->sheet[0].height = 0;
    ss->sheet[0].buf = (unsigned char *)malloc_4k(ss->vxs * ss->vys);
    ss->h[0] = &ss->sheet[0];

    // 初始化鼠标图层
    ss->sheet[1].flags = 1;
    ss->sheet[1].vx0 = ss->vxs/2;
    ss->sheet[1].vy0 = ss->vys/2;
    ss->sheet[1].xs = MOUSEX;
    ss->sheet[1].ys = MOUSEY;
    ss->sheet[1].height = 1;
    ss->sheet[1].buf = (unsigned char *)MOUSE.cursor;
    ss->h[1] = &ss->sheet[1];

    ss->top = 1;
    for (int i = 2; i < MAX_SHEETS; i++) {
        ss->sheet[i].flags = 0;         // 其余标记为未使用
    }

    init_screen(ss->h[0]->buf, ss->h[0]->xs, ss->h[0]->ys);         // 绘制背景
    init_cursor(ss->h[ss->top]->buf);                               // 绘制光标
    sheet_refreshmap(0, 0, 0, ss->vxs, ss->vys);
    sheet_refreshsub(0, ss->top, 0, 0, ss->vxs, ss->vys);
}

struct SHEET *sheet_alloc(int vx0, int vy0, int xs, int ys, unsigned char *buf) {
    struct SHEET *sht;

    for (int i = 0; i < MAX_SHEETS; i++) {
        if (SHEETS.sheet[i].flags == 0) {
            sht = &SHEETS.sheet[i];
            sht->flags = 1;             // 标记为正在使用
            sht->height = 0;            // 高度0隐藏状态
            break;
        }
    }

    sht->vx0 = vx0;
    sht->vy0 = vy0;
	sht->xs = xs;
	sht->ys = ys;
    sht->buf = buf;
    return sht;
}

void sheet_updown(struct SHEET *sht, unsigned char height) {
    unsigned char h, old = sht->height;     // 保存设置前的高度信息

    if (height == old) return;
    if (height > SHEETS.top) height = SHEETS.top;
    sht->height = height;                   // 设定高度

    // 对SHEETS.sht[]重新排序
    if (height < old) {
        if (height) {                       // 降低图层
            for (h = old; h > height; h--) {
                SHEETS.h[h] = SHEETS.h[h - 1];
                SHEETS.h[h]->height = h;
            }
            SHEETS.h[height] = sht;
            sheet_refreshmap(height, sht->vx0, sht->vy0, sht->xs, sht->ys);
            sheet_refreshsub(height, old, sht->vx0, sht->vy0, sht->xs, sht->ys);
        } else {                            // 隐藏图层
            for (h = old; h < SHEETS.top; h++) {
                SHEETS.h[h] = SHEETS.h[h + 1];
                SHEETS.h[h]->height = h;
            }
            SHEETS.top--;
            sheet_refreshmap(0, sht->vx0, sht->vy0, sht->xs, sht->ys);
            sheet_refreshsub(0, old-1, sht->vx0, sht->vy0, sht->xs, sht->ys);
        }
    } else {
        if (old) {                          // 提升图层
            for (h = old; h < height; h++) {
                SHEETS.h[h] = SHEETS.h[h + 1];
                SHEETS.h[h]->height = h;
            }
            SHEETS.h[height] = sht;
        } else {                            // 由隐藏转为显示状态
            for (h = SHEETS.top; h >= height; h--) {
                SHEETS.h[h+1] = SHEETS.h[h];
                SHEETS.h[h+1]->height = h+1;
            }
            SHEETS.h[height] = sht;
            SHEETS.top++;
        }
        sheet_refreshmap(height, sht->vx0, sht->vy0, sht->xs, sht->ys);
        sheet_refreshsub(height, height, sht->vx0, sht->vy0, sht->xs, sht->ys);
    }
}

void sheet_refreshmap(int h0, int vx0, int vy0, int xs, int ys) {
    int vx1 = vx0 + xs, vy1 = vy0 + ys;
    int bx0, by0, bx1, by1, vx, vy;
    unsigned char *buf;
    struct SHEET *sht;
    if(vx0 < 0) vx0 = 0;
    if(vy0 < 0) vy0 = 0;
    if(vx1 > SHEETS.vxs) vx1 = SHEETS.vxs;
    if(vy1 > SHEETS.vys) vy1 = SHEETS.vys;
    for (int h = h0; h <= SHEETS.top; h++) {
        sht = SHEETS.h[h];
        buf = sht->buf;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) bx0 = 0;
        if (by0 < 0) by0 = 0;
        if (bx1 > sht->xs) bx1 = sht->xs;
        if (by1 > sht->ys) by1 = sht->ys;
        for (int by = by0; by < by1; by++) {
            vy = sht->vy0 + by;
            for (int bx = bx0; bx < bx1; bx++) {
                vx = sht->vx0 + bx;
                if (buf[by * sht->xs + bx] != COL_INVISIBLE) {
                    SHEETS.vmap[vy * SHEETS.vxs + vx] = h;
                }
            }
        }
    }
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bxs, int bys) {
    int vx0 = sht->vx0 + bx0, vy0 = sht->vy0 + by0;
    sheet_refreshsub(sht->height, sht->height, vx0, vy0, bxs, bys);
}

void sheet_refreshsub(int h0, int h1, int vx0, int vy0, int xs, int ys) {
    int vx1 = vx0 + xs, vy1 = vy0 + ys;
    int bx0, by0, bx1, by1, vx, vy;
    unsigned char *buf;
    struct SHEET *sht;
    if(vx0 < 0) vx0 = 0;
    if(vy0 < 0) vy0 = 0;
    if(vx1 > SHEETS.vxs) vx1 = SHEETS.vxs;
    if(vy1 > SHEETS.vys) vy1 = SHEETS.vys;
    for (int h = h0; h <= h1; h++) {
        sht = SHEETS.h[h];
        buf = sht->buf;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) bx0 = 0;
        if (by0 < 0) by0 = 0;
        if (bx1 > sht->xs) bx1 = sht->xs;
        if (by1 > sht->ys) by1 = sht->ys;
        for (int by = by0; by < by1; by++) {
            vy = sht->vy0 + by;
            for (int bx = bx0; bx < bx1; bx++) {
                vx = sht->vx0 + bx;
                if (SHEETS.vmap[vy * SHEETS.vxs + vx] == h) {
                    SHEETS.vram[vy * SHEETS.vxs + vx] = buf[by * sht->xs + bx];
                }
            }
        }
    }
}

void sheet_slide(struct SHEET *sht, int dx, int dy) {
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    int new_vx0 = sht->vx0 + dx, new_vy0 = sht->vy0 + dy;
    if(sht->height == SHEETS.top) {
        if(new_vx0 < 0) new_vx0 = 0;
        if(new_vy0 < 0) new_vy0 = 0;
        if(new_vx0 > SHEETS.vxs - 1) new_vx0 = SHEETS.vxs - 1;
        if(new_vy0 > SHEETS.vys - 1) new_vy0 = SHEETS.vys - 1;
    }
    sht->vx0 = new_vx0;
    sht->vy0 = new_vy0;
    if (sht->height) {
        sheet_refreshmap(0, old_vx0, old_vy0, sht->xs, sht->ys);
        sheet_refreshmap(sht->height, new_vx0, new_vy0, sht->xs, sht->ys);
        sheet_refreshsub(0, sht->height-1, old_vx0, old_vy0, sht->xs, sht->ys);
        sheet_refreshsub(sht->height, sht->height, new_vx0, new_vy0, sht->xs, sht->ys);
    }
}

void sheet_free(struct SHEET *sht) {
    if (sht->height) sheet_updown(sht, 0);
    sht->flags = 0;
}

void sheet_make_window(struct SHEET *sht, char *title) {
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };

    int xs = sht->xs, ys = sht->ys;
    boxfill(sht->buf, xs, 0,    0,      xs,     1,      COL8_C6C6C6);
    boxfill(sht->buf, xs, 1,    1,      xs-1,   2,      COL8_FFFFFF);
    boxfill(sht->buf, xs, 0,    0,      1,      ys,     COL8_C6C6C6);
    boxfill(sht->buf, xs, 1,    1,      2,      ys-1,   COL8_FFFFFF);
    boxfill(sht->buf, xs, xs-2, 1,      xs-1,   ys-1,   COL8_848484);
    boxfill(sht->buf, xs, xs-1, 0,      xs,     ys,     COL8_000000);
    boxfill(sht->buf, xs, 2,    2,      xs-2,   ys-2,   COL8_C6C6C6);
    boxfill(sht->buf, xs, 3,    3,      xs-3,   21,     COL8_000084);
    boxfill(sht->buf, xs, 1,    ys-2,   xs-1,   ys-1,   COL8_848484);
    boxfill(sht->buf, xs, 0,    ys-1,   xs,     ys,     COL8_000000);
    putstr8(sht->buf, xs, 24, 4, COL8_FFFFFF, title);

    char c;
    for (int y = 0; y < 14; y++) {
        for (int x = 0; x < 16; x++) {
            c = closebtn[y][x];
            switch (c)
            {
            case '@':
                c = COL8_000000;
                break;
            case '$':
                c = COL8_848484;
                break;
            case 'Q':
                c = COL8_C6C6C6;
                break;
            default:
                c = COL8_FFFFFF;
                break;
            }
            sht->buf[(5 + y) * xs + (xs - 21 + x)] = c;
        }
    }
}

void sheet_make_textbox(struct SHEET *sht, int bx0, int by0, int bxs, int bys, int c) {
	int bx1 = bx0 + bxs, by1 = by0 + bys;
	boxfill(sht->buf, sht->xs, bx0 - 2, by0 - 3, bx1 + 2, by0 - 2, COL8_848484);
	boxfill(sht->buf, sht->xs, bx0 - 3, by0 - 3, bx0 - 2, by1 + 2, COL8_848484);
	boxfill(sht->buf, sht->xs, bx0 - 3, by1 + 2, bx1 + 2, by1 + 3, COL8_FFFFFF);
	boxfill(sht->buf, sht->xs, bx1 + 2, by0 - 3, bx1 + 3, by1 + 3, COL8_FFFFFF);
	boxfill(sht->buf, sht->xs, bx0 - 1, by0 - 2, bx1 + 1, by0 - 1, COL8_000000);
	boxfill(sht->buf, sht->xs, bx0 - 2, by0 - 2, bx0 - 1, by1 + 1, COL8_000000);
	boxfill(sht->buf, sht->xs, bx0 - 2, by1 + 1, bx1 + 1, by1 + 2, COL8_C6C6C6);
	boxfill(sht->buf, sht->xs, bx1 + 1, by0 - 2, bx1 + 2, by1 + 2, COL8_C6C6C6);
	boxfill(sht->buf, sht->xs, bx0 - 1, by0 - 1, bx1 + 1, by1 + 1, c);
}

void sheet_putstr(struct SHEET *sht, int bx0, int by0, char *s, int l, int b, int c) {
    boxfill(sht->buf, sht->xs, bx0, by0, bx0+l*8, by0+16, b);
    putstr8(sht->buf, sht->xs, bx0, by0, c, s);
    sheet_refresh(sht, bx0, by0, l*8, 16);
}
