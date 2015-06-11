#ifndef __KERNDATA_H__
#define __KERNDATA_H__

#include <asm/mmu.h>
#include <asm/e820.h>
#include <kernel/mm.h>

extern uint64_t boot_gdt[N_BOOT_GDT_ENTRY];
extern struct gdt_ptr boot_gdtptr;
extern struct mem_e820_map e820map;

enum {
    KERNDATA_NONE,
    KERNDATA_BOOTGDT,
    KERNDATA_BOOTGDTPTR,
    KERNDATA_E820
};

#endif
