
#include "dogos.h"

static unsigned char charset[16 * 8] = {
    /* invader(0) */
    0x00, 0x00, 0x00, 0x43, 0x5f, 0x5f, 0x5f, 0x7f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x20, 0x3f, 0x00,

    /* invader(1) */
    0x00, 0x0f, 0x7f, 0xff, 0xcf, 0xcf, 0xcf, 0xff,
    0xff, 0xe0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0x00,

    /* invader(2) */
    0x00, 0xf0, 0xfe, 0xff, 0xf3, 0xf3, 0xf3, 0xff,
    0xff, 0x07, 0xff, 0xff, 0x03, 0x03, 0x03, 0x00,

    /* invader(3) */
    0x00, 0x00, 0x00, 0xc2, 0xfa, 0xfa, 0xfa, 0xfe,
    0xf8, 0xf8, 0xf8, 0xf8, 0x00, 0x04, 0xfc, 0x00,

    /* fighter(0) */
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x43, 0x47, 0x4f, 0x5f, 0x7f, 0x7f, 0x00,

    /* fighter(1) */
    0x18, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff,
    0xff, 0xff, 0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0x00,

    /* fighter(2) */
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0xc2, 0xe2, 0xf2, 0xfa, 0xfe, 0xfe, 0x00,

    /* laser */
    0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00
};
/* invader:"abcd", fighter:"efg", laser:"h" */

static char invstr0[32] = " abcd abcd abcd abcd abcd ";

void putstr(struct SHEET *sht, int bx0, int by0, int col, char *s);

void Task_invader(void) {
    // 绘制图层
    unsigned char *sht_buf = (unsigned char *)malloc_4k(336 * 261);
    struct SHEET *sht = sheet_alloc((SHEETS.vxs-336)/2, (SHEETS.vys-261)/2, 336, 261, sht_buf, TASKS.now);
    sheet_make_window(sht, "invader [keynote: a, d, space]");
    boxfill(sht->buf, sht->xs, 6, 27, 329, 254, 0);
    sheet_updown(sht, SHEETS.top);

    // 设置定时器
    struct FIFO8 timerfifo;
    char timerbuf[8];
    fifo8_init(&timerfifo, 8, timerbuf, TASKS.now);
    struct TIMER *t_invader = timer_alloc();
    timer_init(t_invader, &timerfifo, 1);
    struct TIMER *t_laser = timer_alloc();
    timer_init(t_laser, &timerfifo, 2);

    int fx;                 // 自机的x坐标

    int lx, ly = 0;         // 炮弹坐标

    int ix, iy;             // 外星人群坐标
    int invline;            // 外星人群行数
    int idir;               // 外星人移动方向
    char invstr[32 * 6];    // 外星人状态

    int score;              // 当前得分
    char s[12], *p;

    for (int i = 0; i < 14; i++) {
		putstr(sht, 0, i, 0, "                                        ");
	}

    score = 0;
    putstr(sht, 4, 0, 7, "SCORE:00000000");

    fx = 18;
    putstr(sht, fx, 13, 6, "efg");
    timer_settime(t_invader, 20);

next_group:
    ix = 7;
    iy = 1;
    invline = 6;
    idir = +1;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 27; j++) {
            invstr[i * 32 + j] = invstr0[j];
        }
        putstr(sht, ix, iy + i, 2, invstr + i * 32);
    }

    timer_settime(t_laser, 5);

    for (;;) {
        task_sleep(TASKS.now);

        // 键盘中断响应
        while((sht->height == SHEETS.top-1) && fifo8_status(&KEYBOARD.fifo)) {
            io_cli();
            unsigned char byte_ascii = fifo8_get(&KEYBOARD.fifo);
            io_sti();

            switch(byte_ascii) {
            case 'a':   // 左移
                if(fx > 0) {
                    fx--;
                    putstr(sht, fx, 13, 6, "efg ");
                }
                break;
            case 'd':   // 右移
                if(fx < 37) {
                    putstr(sht, fx, 13, 6, " efg");
                    fx++;
                }
                break;
            case ' ':   // 发射炮弹
                if(ly == 0) {
			        lx = fx + 1;
			        ly = 13;
                }
                break;
            default:    // 其它按键
                break;
            }
        }

        // 定时器超时响应
        while(fifo8_status(&timerfifo)) {
            io_cli();
            unsigned char timer = fifo8_get(&timerfifo);
            io_sti();
            
            switch(timer) {
            case 1:     // t_invader
                if (ix + idir > 14 || ix + idir < 0) {
				    if (iy + invline == 13) {
					    putstr(sht, 15, 6, 1, "GAME OVER");
	                    goto game_over;
				    }
				    idir = - idir;
				    putstr(sht, ix + 1, iy, 0, "                         ");
				    iy++;
			    } else {
				    ix += idir;
			    }
			    for (int i = 0; i < invline; i++) {
				    putstr(sht, ix, iy + i, 2, invstr + i * 32);
			    }

                timer_settime(t_invader, 20);
                break;
            case 2:     // t_laser
                if (ly) {
                    if(ly < 13) putstr(sht, lx, ly, 0, " ");
                    ly--;
                    if(ly > 0) putstr(sht, lx, ly, 3, "h");
                }

			    if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invline) {
				    p = invstr + (ly - iy) * 32 + (lx - ix);
				    if (*p != ' ') {    // 命中
					    score += 1;
					    sprintf(s, "%08d", score);
					    putstr(sht, 10, 0, 7, s);

					    while(*p != ' ') p--;
					    for(int i = 1; i < 5; i++) p[i] = ' ';
					    putstr(sht, ix, ly, 2, invstr + (ly - iy) * 32);

					    for (; invline > 0; invline--) {
						    for (p = invstr + (invline - 1) * 32; *p != 0; p++) {
							    if (*p != ' ') {
								    goto alive;
							    }
						    }
					    }
					    // 全部消灭
                        ly = 0;
					    goto next_group;
	                alive:
					    ly = 0;
				    }
			    }

                timer_settime(t_laser, 5);
                break;
            default:    // 其它按键
                break;
            }
        }
	}

game_over:  // do nothing
    for (;;) {
        task_sleep(TASKS.now);

        while((sht->height == SHEETS.top-1) && fifo8_status(&KEYBOARD.fifo)) {
            io_cli();
            unsigned char byte_ascii = fifo8_get(&KEYBOARD.fifo);
            io_sti();
        }
    }
}

// bx0=0~39 by0=0~13 
void putstr(struct SHEET *sht, int bx0, int by0, int col, char *s) {
    int bx = bx0 * 8 + 8;
    int by = by0 * 16 + 29;
    int i = strlen(s);
    boxfill(sht->buf, sht->xs, bx, by, bx + i * 8 - 1, by + 15, 0);

    unsigned char *p, *q = sht->buf + by * 336;
    char c, t[2];
    t[1] = 0;

    for (;;) {
        c = *s;
        if (c == 0) break;
        if (c != ' ') {
            if ('a' <= c && c <= 'h') {
                p = charset + 16 * (c - 'a');
                q += bx;
                for (i = 0; i < 16; i++) {
                    if ((p[i] & 0x80) != 0) { q[0] = col; }
                    if ((p[i] & 0x40) != 0) { q[1] = col; }
                    if ((p[i] & 0x20) != 0) { q[2] = col; }
                    if ((p[i] & 0x10) != 0) { q[3] = col; }
                    if ((p[i] & 0x08) != 0) { q[4] = col; }
                    if ((p[i] & 0x04) != 0) { q[5] = col; }
                    if ((p[i] & 0x02) != 0) { q[6] = col; }
                    if ((p[i] & 0x01) != 0) { q[7] = col; }
                    q += 336;
                }
                q -= 336 * 16 + bx;
            } else {
                t[0] = *s;
                sheet_putstr(sht, bx, by, t, 1, 0, col);
            }
        }
        s++;
        bx += 8;
    }
    sheet_refresh(sht, bx0*8+8, by, bx, by + 16);
}
