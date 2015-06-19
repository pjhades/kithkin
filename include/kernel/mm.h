#ifndef __MM_H__
#define __MM_H__

#include <asm/mmu.h>

#define KERNEL_VM_START     0xc0000000
#define KERNEL_STARTUP_DATA 0x106000

#define N_PDE         1024
#define N_PTE_PER_PDE 1024
#define N_USER_PDE    (KERNEL_VM_START >> PGDIR_SHIFT)
#define N_KERNEL_PDE  (N_PDE - N_USER_PDE)

#define PGDIR_SHIFT 22
#define PAGE_SHIFT  12
#define PAGE_SIZE   (1 << PAGE_SHIFT)


#ifndef __ASSEMBLER__
#include <list.h>

#define MIN_PHYS 0x100000

#define phys_to_pfn(pa)  ((pa) >> PAGE_SHIFT)
#define pfn_to_phys(pfn) ((pfn) << PAGE_SHIFT)

typedef uint32_t pde_t;
typedef uint32_t pte_t;

struct gdt_ptr {
    uint16_t len;
    uint32_t ptr;
} __attribute__((packed));

extern uint32_t minpfn, maxpfn;

struct page {
    uint32_t flags;
    struct list_node lru;
};

void meminit(void);
#endif

#endif
