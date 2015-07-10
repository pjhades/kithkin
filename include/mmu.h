#ifndef __MMU_H__
#define __MMU_H__

#ifndef __ASSEMBLER__
#include <kernel/types.h>

#define seg_desc(base, limit, flags)   \
    (((((u64)base )&0xff000000)<<32) | \
     ((((u64)flags)&0x0000f0ff)<<40) | \
     ((((u64)limit)&0x000f0000)<<32) | \
     ((((u64)base )&0x00ffffff)<<16) | \
      (((u64)limit)&0x0000ffff))

struct gdt_ptr {
    u16 len;
    u32 ptr;
} __attribute__((packed));

#endif /* ASSEMBLER */

#define CR0_PM  0x1
#define CR0_PG  0x80000000

#endif
