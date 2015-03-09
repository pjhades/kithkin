#ifndef __MM_H__
#define __MM_H__

#include <asm/mmu.h>

#define KERNEL_VM_START 0xc0000000
#define PGDIR_SHIFT     22
#define N_PDE           1024
#define N_PTE_PER_PDE   1024
#define N_USER_PDE      (KERNEL_VM_START >> PGDIR_SHIFT)
#define N_KERNEL_PDE    (N_PDE - N_USER_PDE)

#ifndef __ASSEMBLER__
typedef uint32_t pde_t;
typedef uint32_t pte_t;

struct gdt_ptr {
    uint16_t len;
    uint32_t ptr;
} __attribute__((packed));
#endif

#endif
