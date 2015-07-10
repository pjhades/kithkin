#ifndef __MM_H__
#define __MM_H__

#define KERNEL_VIRT_START 0xc0000000

#define N_USER_PDE    (KERNEL_VIRT_START >> PAGEDIR_SHIFT)
#define N_KERNEL_PDE  (N_PDE - N_USER_PDE)

#ifndef __ASSEMBLER__

#include <paging.h>
#include <list.h>
#include <kernel/types.h>

/* we do not touch physical memory lower than this */
#define MIN_PHYS 0x100000

/* directly map the first 896MB to kernel virtual space */
#define DIRECTMAP_PHYS_MAX 0x38000000

#define set_pde(addr, pde) *((pde_t *)addr) = (pde_t)(pde)
#define set_pte(addr, pte) *((pte_t *)addr) = (pte_t)(pte)

#define phys_to_pfn(pa)  ((u32)(pa) >> PAGE_SHIFT)
#define pfn_to_phys(pfn) ((u32)(pfn) << PAGE_SHIFT)

#define pfn_up(pa) (((u32)(pa) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define pfn_down(pa) ((u32)(pa) >> PAGE_SHIFT)

#define phys(addr) ((u32)(addr) - KERNEL_VIRT_START)
#define virt(addr) ((u32)(addr) + KERNEL_VIRT_START)

#define page_to_idx(page) ((page) - mem_map)
#define idx_to_page(idx) ((idx) + mem_map)

#define pfn_to_page(pfn) idx_to_page((pfn) - minpfn)
#define page_to_pfn(page) (page_to_idx((page)) + minpfn)

typedef u32 pde_t;
typedef u32 pte_t;

enum {
    PG_BUDDY,
};

#define page_order(page) (int)((page)->data)
#define set_page_order(page, order) (page)->data = (void *)(order)

extern char _end[];
extern u32 minpfn, maxpfn;
extern struct page *mem_map;

struct page {
    u32 flags;
    void *data;
    struct list_node lru;
} __attribute__((packed));

void meminit(void);

#endif /* ASSEMBLER */

#endif
