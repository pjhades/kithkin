#ifndef __MMU_H__
#define __MMU_H__

/* segment descriptor */
#define SEG_DESC(base, limit, flags)        \
    (((((uint64_t)base )&0xff000000)<<32) | \
     ((((uint64_t)flags)&0x0000f0ff)<<40) | \
     ((((uint64_t)limit)&0x000f0000)<<32) | \
     ((((uint64_t)base )&0x00ffffff)<<16) | \
      (((uint64_t)limit)&0x0000ffff))

#define PDE_P   0x1
#define PDE_RW  0x2
#define PDE_US  0x4
#define PDE_PWT 0x8
#define PDE_PCD 0x10
#define PDE_A   0x20

#define PTE_P   0x1
#define PTE_RW  0x2
#define PTE_US  0x4
#define PTE_PWT 0x8
#define PTE_PCD 0x10
#define PTE_A   0x20
#define PTE_D   0x40
#define PTE_PAT 0x80
#define PTE_G   0x100

#endif
