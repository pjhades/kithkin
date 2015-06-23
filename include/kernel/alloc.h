#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <kernel/types.h>

struct bootmem_data {
    char *bitmap;     /* usable or reserved */
    uint32_t size;    /* bitmap size */
    void *last;       /* continue next allocation from here ... */
    uint32_t lastpfn; /* ... and this page frame */
};

#define bootmem_mark_usable(bdata, pfn) \
    bdata.bitmap[pfn>>3] |= (1 << (pfn & 7))
#define bootmem_mark_reserved(bdata, pfn) \
    bdata.bitmap[pfn>>3] &= ~(1 << (pfn & 7))

void init_bootmem(void);
void *bootmem_alloc(uint32_t size);

#endif
