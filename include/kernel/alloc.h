#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <kernel/types.h>

struct bootmem_data {
    char *bitmap;
    uint32_t size;
    char *last;
    uint32_t lastpfn;
};

#define bootmem_mark_usable(bdata, pfn) \
    bdata.bitmap[pfn>>3] |= (1 << (pfn & 7))
#define bootmem_mark_reserved(bdata, pfn) \
    bdata.bitmap[pfn>>3] &= ~(1 << (pfn & 7))

void init_bootmem(void);

#endif
