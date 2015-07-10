#ifndef __BOOTDATA_H__
#define __BOOTDATA_H__

#include <mmu.h>
#include <e820.h>
#include <boot.h>

/* physical address of data obtained before kernel executes */
#define KERNEL_BOOTDATA 0x00007e00

extern u64 boot_gdt[N_BOOT_GDT_ENTRY];
extern struct gdt_ptr boot_gdtptr;
extern struct mem_e820_map e820map;

enum {
    BOOTDATA_NONE,
    BOOTDATA_BOOTGDT,
    BOOTDATA_BOOTGDTPTR,
    BOOTDATA_E820
};

#endif
