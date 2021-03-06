#ifndef __MM_H__
#define __MM_H__

#define KERNEL_BASE_VA 0xc0000000

#define NR_USER_PDE    (KERNEL_BASE_VA >> PAGEDIR_SHIFT)
#define NR_KERNEL_PDE  (NR_PDE - NR_USER_PDE)

#ifndef __ASSEMBLER__

#include <paging.h>
#include <list.h>
#include <kernel/types.h>

/* we do not touch physical memory lower than this */
#define MIN_PA 0x100000

/* directly map the first 896MB to kernel virtual space */
#define DIRECT_MAP_MAX_PA 0x38000000

#define set_pde(addr, pde) *((pde_t *)addr) = (pde_t)(pde)
#define set_pte(addr, pte) *((pte_t *)addr) = (pte_t)(pte)

#define phys_to_pfn(pa)  ((u32)(pa) >> PAGE_SHIFT)
#define pfn_to_phys(pfn) ((u32)(pfn) << PAGE_SHIFT)

#define pfn_up(pa) (((u32)(pa) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define pfn_down(pa) ((u32)(pa) >> PAGE_SHIFT)

#define phys(addr) ((u32)(addr) - KERNEL_BASE_VA)
#define virt(addr) ((u32)(addr) + KERNEL_BASE_VA)

#define page_to_idx(page) ((page) - mem_map)
#define idx_to_page(idx) ((idx) + mem_map)

#define pfn_to_page(pfn) idx_to_page((pfn) - minpfn)
#define page_to_pfn(page) (page_to_idx((page)) + minpfn)

#define direct_map_page_to_virt(page) virt(pfn_to_phys(page_to_pfn(page)))

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
