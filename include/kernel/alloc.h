#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <kernel/types.h>

struct bootmem_data {
    unsigned char *bitmap; /* 0 usable, 1 reserved */
    uint32_t size;         /* bitmap size */
    void *last;            /* continue next allocation from here ... */
    uint32_t lastpfn;      /* ... and this page frame */
};

void init_bootmem(void);
void *bootmem_alloc(uint32_t size);

#endif
