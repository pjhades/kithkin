#ifndef __BOOTDATA_H__
#define __BOOTDATA_H__

#include <asm/mmu.h>
#include <asm/e820.h>
#include <kernel/mm.h>

extern uint64_t boot_gdt[N_BOOT_GDT_ENTRY];
extern struct gdt_ptr boot_gdtptr;
extern struct mem_e820_map e820map;

enum {
    BOOTDATA_NONE,
    BOOTDATA_BOOTGDT,
    BOOTDATA_BOOTGDTPTR,
    BOOTDATA_E820
};

#endif
