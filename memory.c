
#include "dogos.h"

struct MEMORYCTL MEMORY;

/* 内存容量测试 */
int mtest(unsigned long start, unsigned long end) {
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

	return i>>20;
}

void init_memory(struct MEMORYCTL *mm) {
    int i = 0;
    for(; i < 64; i++) mm->bitmap[i] = 0xff;
    for(; i < 256; i++) mm->bitmap[i] = 0x00;
    mm->total = mtest(0x00400000, 0xbfffffff);
    mm->free = ((mm->total<8)?(mm->total-2):(8-2))*1024/4;
}

unsigned long malloc_4k(unsigned int size) {
    unsigned int m = 0, n = ((size + 0xfff) & 0xfffff000) >> 12;
    unsigned long i, p, q;
    for(i = 64*8; i < 256*8; i++) {
        p = i / 8;
        q = i % 8;
        if(MEMORY.bitmap[p] & (1<<q)) {
            m = 0;
        } else {
            m++;
        }
        if(m == n) break;
    }

    if(i >= 256*8) return 0;
    MEMORY.free -= n;

    i = i - n + 1;
    unsigned long addr = i << 12;
    n = i + n;
    for(; i < n; i++) {
        p = i / 8;
        q = i % 8;
        MEMORY.bitmap[p] |= 1<<q;
    }
    return addr;
}

int mfree_4k(unsigned long addr, unsigned int size) {
    unsigned int n = ((size + 0xfff) & 0xfffff000) >> 12;
    unsigned long p;
    unsigned long q;
    addr >>= 12;
    for(int i = 0; i < n; i++) {
        p = addr / 8;
        q = addr % 8;
        if(MEMORY.bitmap[p] & (1<<q)) {
            MEMORY.bitmap[p] ^= 1<<q;
            MEMORY.free++;
        } else {
            return -1;
        }
        addr++;
    }
    return 0;
}
