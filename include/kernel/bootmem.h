#ifndef __BOOTMEM_H__
#define __BOOTMEM_H__

#include <kernel/types.h>

struct bootmem_data {
    unsigned char *bitmap; /* 0 usable, 1 reserved */
    u32 size;         /* bitmap size */
    void *last;            /* continue next allocation from here ... */
    u32 lastpfn;      /* ... and this page frame */
};

void init_bootmem(void);
void *bootmem_alloc(u32 size);
void free_all_bootmem(void);

#endif
