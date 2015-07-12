#ifndef __SAVED_DATA_H__
#define __SAVED_DATA_H__

#include <seg.h>
#include <e820.h>
#include <pm.h>

/* physical address of data obtained before kernel executes */
#define KERNEL_SAVED_DATA 0x00007e00

extern u64 boot_gdt[NR_BOOT_GDT_ENTRY];
extern struct gdtptr boot_gdtptr;
extern struct mem_e820_map e820map;

enum {
    SAVED_DATA_NONE,
    SAVED_DATA_BOOTGDT,
    SAVED_DATA_BOOTGDTPTR,
    SAVED_DATA_E820
};

#endif
