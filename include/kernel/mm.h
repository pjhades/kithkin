#ifndef __MM_H__
#define __MM_H__

#include <asm/mmu.h>

/* we do not touch physical memory lower than this */
#define MIN_PHYS 0x100000

#define KERNEL_VIRT_START 0xc0000000

/* physical address of data obtained before kernel executes */
#define KERNEL_BOOTDATA 0x00007e00

/* directly map the first 896MB to kernel virtual space */
#define DIRECTMAP_PHYS_MAX 0x38000000

#define PAGEDIR_SHIFT 22
#define PAGE_SHIFT    12
#define PAGE_SIZE     (1 << PAGE_SHIFT)
#define PAGE_MASK     (PAGE_SIZE - 1)
#define N_PDE         1024
#define N_PTE_PER_PDE 1024
#define N_USER_PDE    (KERNEL_VIRT_START >> PAGEDIR_SHIFT)
#define N_KERNEL_PDE  (N_PDE - N_USER_PDE)

#define set_pde(addr, pde) *((pde_t *)addr) = (pde_t)(pde)
#define set_pte(addr, pte) *((pte_t *)addr) = (pte_t)(pte)

#define phys_to_pfn(pa)  ((u32)(pa) >> PAGE_SHIFT)
#define pfn_to_phys(pfn) ((u32)(pfn) << PAGE_SHIFT)

#define pfn_up(pa) (((u32)(pa) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define pfn_down(pa) ((u32)(pa) >> PAGE_SHIFT)

#define phys(addr) ((u32)(addr) - KERNEL_VIRT_START)
#define virt(addr) ((u32)(addr) + KERNEL_VIRT_START)

#define pfn_to_page(pfn) (mem_map + (pfn) - minpfn)
#define page_to_pfn(page) ((page) - mem_map + minpfn)


#ifndef __ASSEMBLER__
#include <list.h>
extern u32 minpfn, maxpfn;

typedef u32 pde_t;
typedef u32 pte_t;

struct page {
    struct list_node lru;
};

void meminit(void);

#endif

#endif
