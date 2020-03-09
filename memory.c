
#include "dogos.h"

struct MEMMAN MEMORY;

/* 内存容量测试 */
unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0;

    // 确定CPU是386还是486以上
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
	if (eflg & EFLAGS_AC_BIT) flg486 = 1;   // 386的AC位还是会等于0
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if (flg486) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;           // 禁止缓存
		store_cr0(cr0);
	}

	unsigned long i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for (i = start; i <= end; i += 0x1000) {
		p = (unsigned long *)(i + 0xffc);
		old = *p;                           // 保存旧值
		*p = pat0;                          // 试写
		*p ^= 0xffffffff;                   // 反转
		if (*p != pat1) break;              // 检查反转值
		*p ^= 0xffffffff;                   // 再次反转
		if (*p != pat0) break;              // 检查恢复值
		*p = old;                           // 恢复旧值
	}

	if (flg486) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;          // 允许缓存
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man) {
    man->frees = 0;                         // 可用信息数目
    man->maxfrees = 0;
    man->lostsize = 0;                      // 释放失败内存大小
    man->losts = 0;                         // 释放失败次数
}

/* 报告空闲内存大小 */
unsigned int memman_total(struct MEMMAN *man) {
    unsigned int i, t = 0;
    for (i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
    unsigned int i, a;
    for (i = 0; i < man->frees; i++) {      // 找到足够大的空闲内存
        if (man->free[i].size >= size) {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;                       // 可用空间地址
        }
    }

    return 0;                               // 没有可用空间
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
    int i, j;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) break;
    }

    // free[i - 1].addr <= addr < free[i].addr
    if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
        man->free[i - 1].size += size;      // 与前一项合并
        if (i < man->frees) {
            if (addr + size == man->free[i].addr) {
                man->free[i - 1].size += man->free[i].size;
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1];
                }
            }
        }
        return 0;
	}

    if (i < man->frees) {
        if (addr + size == man->free[i].addr) {
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }

    if (man->frees < MEMMAN_FREES) {
        for (j = man->frees; j > i; j--) man->free[j] = man->free[j - 1];
        man->frees++;
        if (man->maxfrees < man->frees) man->maxfrees = man->frees;
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }

    // 释放失败
    man->losts++;
    man->lostsize += size;
    return -1;
}
