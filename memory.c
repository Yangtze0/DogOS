
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
    MEMORY.frees = 0;                         // 可用信息数目
    MEMORY.maxfrees = 0;
    MEMORY.lostsize = 0;                      // 释放失败内存大小
    MEMORY.losts = 0;                         // 释放失败次数
}

/* 报告空闲内存大小 */
unsigned int memman_total(void) {
    unsigned int i, t = 0;
    for (i = 0; i < MEMORY.frees; i++) {
        t += MEMORY.free[i].size;
    }
    return t;
}

unsigned int memman_alloc(unsigned int size) {
    unsigned int i, a;
    for (i = 0; i < MEMORY.frees; i++) {      // 找到足够大的空闲内存
        if (MEMORY.free[i].size >= size) {
            a = MEMORY.free[i].addr;
            MEMORY.free[i].addr += size;
            MEMORY.free[i].size -= size;
            if (MEMORY.free[i].size == 0) {
                MEMORY.frees--;
                for (; i < MEMORY.frees; i++) {
                    MEMORY.free[i] = MEMORY.free[i + 1];
                }
            }
            return a;                       // 可用空间地址
        }
    }

    return 0;                               // 没有可用空间
}

int memman_free(unsigned int addr, unsigned int size) {
    int i, j;
    for (i = 0; i < MEMORY.frees; i++) {
        if (MEMORY.free[i].addr > addr) break;
    }

    // free[i - 1].addr <= addr < free[i].addr
    if (MEMORY.free[i - 1].addr + MEMORY.free[i - 1].size == addr) {
        MEMORY.free[i - 1].size += size;      // 与前一项合并
        if (i < MEMORY.frees) {
            if (addr + size == MEMORY.free[i].addr) {
                MEMORY.free[i - 1].size += MEMORY.free[i].size;
                MEMORY.frees--;
                for (; i < MEMORY.frees; i++) {
                    MEMORY.free[i] = MEMORY.free[i + 1];
                }
            }
        }
        return 0;
	}

    if (i < MEMORY.frees) {
        if (addr + size == MEMORY.free[i].addr) {
            MEMORY.free[i].addr = addr;
            MEMORY.free[i].size += size;
            return 0;
        }
    }

    if (MEMORY.frees < MEMMAN_FREES) {
        for (j = MEMORY.frees; j > i; j--) MEMORY.free[j] = MEMORY.free[j - 1];
        MEMORY.frees++;
        if (MEMORY.maxfrees < MEMORY.frees) MEMORY.maxfrees = MEMORY.frees;
        MEMORY.free[i].addr = addr;
        MEMORY.free[i].size = size;
        return 0;
    }

    // 释放失败
    MEMORY.losts++;
    MEMORY.lostsize += size;
    return -1;
}

unsigned int memman_alloc_4k(unsigned int size) {
    return memman_alloc((size + 0xfff) & 0xfffff000);
}

int memman_free_4k(unsigned int addr, unsigned int size) {
    return memman_free(addr, (size + 0xfff) & 0xfffff000);
}
