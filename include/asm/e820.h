#ifndef __E820_H__
#define __E820_H__

struct mem_e820_entry {
    uint64_t base; 
    uint64_t len;
    uint32_t type;
    uint32_t attr;
};

#define MEM_E820_MAX 128
struct mem_e820 {
    uint32_t n_regions;
    struct mem_e820_entry regions[MEM_E820_MAX];
} e820_map;

#endif

