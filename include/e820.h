#ifndef __E820_H__
#define __E820_H__

#include <kernel/types.h>

/* reference: http://wiki.osdev.org/Detecting_Memory_(x86) */
enum {
    E820_USABLE = 1,
    E820_RESERVED,
    E820_ACPI_RECLAIMABLE,
    E820_ACPI_NVS,
    E820_CONTAINING_BAD
};

struct mem_e820_entry {
    u64 base;
    u64 len;
    u32 type;
    u32 attr;
};

#define MEM_E820_MAX 128
struct mem_e820_map {
    u32 nr_regions;
    struct mem_e820_entry regions[MEM_E820_MAX];
};

#endif

