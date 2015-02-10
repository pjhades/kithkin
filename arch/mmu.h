#ifndef __MMU_H__
#define __MMU_H__

/* segment descriptor */
#define SEG_DESC(base, limit, flags)        \
    (((((uint64_t)base )&0xff000000)<<32) | \
     ((((uint64_t)flags)&0x0000f0ff)<<40) | \
     ((((uint64_t)limit)&0x000f0000)<<32) | \
     ((((uint64_t)base )&0x00ffffff)<<16) | \
      (((uint64_t)limit)&0x0000ffff))

#define BOOT_GDT_ENTRY_CODE 1
#define BOOT_GDT_ENTRY_DATA 2

#define BOOT_CS (BOOT_GDT_ENTRY_CODE << 3)
#define BOOT_DS (BOOT_GDT_ENTRY_DATA << 3)

#endif
