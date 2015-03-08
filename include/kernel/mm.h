#ifndef __MM_H__
#define __MM_H__

#include <asm/mmu.h>

typedef uint32_t pde_t;
typedef uint32_t pte_t;

struct gdt_ptr {
    uint16_t len;
    uint32_t ptr;
} __attribute__((packed));

#endif
