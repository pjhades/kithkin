#ifndef __SEG_H__
#define __SEG_H__

#include <kernel/types.h>

#define seg_desc(base, limit, flags)   \
    (((((u64)base )&0xff000000)<<32) | \
     ((((u64)flags)&0x0000f0ff)<<40) | \
     ((((u64)limit)&0x000f0000)<<32) | \
     ((((u64)base )&0x00ffffff)<<16) | \
      (((u64)limit)&0x0000ffff))

struct gdtptr {
    u16 len;
    u32 ptr;
} __attribute__((packed));

#endif
