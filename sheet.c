
#include "dogos.h"

extern unsigned char *VRAM;
struct SHTCTL SHEETS;

void shtctl_init(struct SHTCTL *ss) {
    ss->top = 0;                        // 无图层
    for (int i = 0; i < MAX_SHEETS; i++) {
        ss->sheets0[i].flags = 0;       // 标记为未使用
    }
}

struct SHEET *sheet_alloc(int x0, int y0, int xs, int ys, unsigned char *buf) {
    struct SHEET *sht;

    for (int i = 0; i < MAX_SHEETS; i++) {
        if (SHEETS.sheets0[i].flags == 0) {
            sht = &SHEETS.sheets0[i];
            sht->flags = 1;             // 标记为正在使用
            sht->height = 0;            // 高度0隐藏状态
            SHEETS.sheets[0] = sht;
            break;
        }
    }

    sht->x0 = x0;
    sht->y0 = y0;
	sht->xs = xs;
	sht->ys = ys;
    sht->buf = buf;
    return sht;
}


void sheet_updown(struct SHEET *sht, unsigned char height) {
    unsigned char h, old = sht->height;     // 保存设置前的高度信息

    if (height > SHEETS.top + 1) height = SHEETS.top + 1;
    sht->height = height;                   // 设定高度

    // 对SHEETS.sheets[]重新排序
    if (height < old) {
        if (height) {
            for (h = old; h > height; h--) {
                SHEETS.sheets[h] = SHEETS.sheets[h - 1];
                SHEETS.sheets[h]->height = h;
            }
            SHEETS.sheets[height] = sht;
        } else {                            // 隐藏图层
            if (SHEETS.top > old) {
                for (h = old; h < SHEETS.top; h++) {
                    SHEETS.sheets[h] = SHEETS.sheets[h + 1];
                    SHEETS.sheets[h]->height = h;
                }
            }
            SHEETS.top--;
        }
    } else if (height > old) {
        if (old) {
            for (h = old; h < height; h++) {
                SHEETS.sheets[h] = SHEETS.sheets[h + 1];
                SHEETS.sheets[h]->height = h;
            }
            SHEETS.sheets[height] = sht;
        } else {                            // 由隐藏转为显示状态
            for (h = SHEETS.top; h >= height; h--) {
                SHEETS.sheets[h + 1] = SHEETS.sheets[h];
                SHEETS.sheets[h + 1]->height = h + 1;
            }
            SHEETS.sheets[height] = sht;
            SHEETS.top++;
        }
    }

    sheet_refresh(sht);                        // 更新画面
}

void sheet_refresh(struct SHEET *sht) {
    sheet_refreshsub(sht->x0, sht->y0, sht->x0 + sht->xs, sht->y0 + sht->ys);
}

void sheet_refreshsub(int x0, int y0, int x1, int y1) {
    int vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, c;
    struct SHEET *sht;
    for (int h = 1; h <= SHEETS.top; h++) {
        sht = SHEETS.sheets[h];
        buf = sht->buf;
        bx0 = x0 - sht->x0;
        by0 = y0 - sht->y0;
        bx1 = x1 - sht->x0;
        by1 = y1 - sht->y0;
        if (bx0 < 0) bx0 = 0;
        if (by0 < 0) by0 = 0;
        if (bx1 > sht->xs) bx1 = sht->xs;
        if (by1 > sht->ys) by1 = sht->ys;
        for (int by = by0; by < by1; by++) {
            vy = sht->y0 + by;
            for (int bx = bx0; bx < bx1; bx++) {
                vx = sht->x0 + bx;
                c = buf[by * sht->xs + bx];
                if (c != COL_INVISIBLE) {
                    VRAM[vy * 320 + vx] = c;
                }
            }
        }
    }
}

void sheet_slide(struct SHEET *sht, int x0, int y0) {
    int old_x0 = sht->x0, old_y0 = sht->y0;
    sht->x0 = x0;
    sht->y0 = y0;
    if (sht->height) {
        sheet_refreshsub(old_x0, old_y0, old_x0 + sht->xs, old_y0 + sht->ys);
        sheet_refreshsub(x0, y0, x0 + sht->xs, y0 + sht->ys);
    }
}

void sheet_free(struct SHEET *sht) {
    if (sht->height) sheet_updown(sht, 0);
    sht->flags = 0;
}