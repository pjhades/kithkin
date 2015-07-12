#include <string.h>
#include <e820.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/saved_data.h>
#include <kernel/mm.h>
#include <kernel/bootmem.h>
#include <kernel/buddy.h>

struct gdtptr gdtptr;
struct mem_e820_map e820map;
u64 boot_gdt[NR_BOOT_GDT_ENTRY];

/* page frame number of the minimal/maximal usable physical page */
u32 minpfn, maxpfn;

struct page *mem_map;

static void get_kernel_data(void)
{
    char *dst = (char *)KERNEL_SAVED_DATA;
    unsigned char type;
    u32 size;

    type = *((unsigned char *)dst);
    while (type != SAVED_DATA_NONE) {
        dst += sizeof(unsigned char);
        size = *((u32 *)dst);
        dst += sizeof(u32);

        if (type == SAVED_DATA_BOOTGDT) {
            memcpy(boot_gdt, dst, size);
            dst += sizeof(u64) * NR_BOOT_GDT_ENTRY;
        }
        else if (type == SAVED_DATA_BOOTGDTPTR) {
            memcpy(&gdtptr, dst, size);
            dst += sizeof(struct gdtptr);
        }
        else if (type == SAVED_DATA_E820) {
            memcpy(&e820map, dst, size);
            dst += sizeof(struct mem_e820_map);
        }
        else
            die("Unknown kernel data type: %x\n", type);

        type = *((unsigned char *)dst);
    }
}

static void scan_e820map(void)
{
    int i;
    u32 maxsize;

    minpfn = 0xffffffff;
    maxpfn = 0;
    maxsize = 0;

    for (i = 0; i < e820map.nr_regions; i++) {
        printk("e820: range 0x%016X - 0x%016X, %s\n", e820map.regions[i].base,
                e820map.regions[i].base + e820map.regions[i].len - 1,
                e820map.regions[i].type == E820_USABLE ? "usable": "reserved");

        if (e820map.regions[i].type != E820_USABLE)
            continue;

        /* here we only use the largest available region */
        if (e820map.regions[i].len < maxsize
                || e820map.regions[i].base < MIN_PA)
            continue;

        minpfn = e820map.regions[i].base;
        maxpfn = e820map.regions[i].base + e820map.regions[i].len - 1;
        maxsize = e820map.regions[i].len;
    }

    if (maxsize == 0)
        die("e820: no minimal usable memory above %x\n", MIN_PA);

    minpfn = pfn_up(minpfn);
    maxpfn = pfn_down(maxpfn);

    if (minpfn > maxpfn)
        die("e820: no usable range found\n");

    printk("e820: minpfn=%p, maxpfn=%p\n", minpfn, maxpfn);
}

static void load_pagetable(pde_t *pagedir)
{
    asm volatile (
            "movl %0, %%cr3\n\t"
            :
            :"a"(pagedir)
            :"memory"
            );
}

static void init_mapping(void)
{
    extern char pagedir[];
    pde_t *pde;
    pte_t *pte;
    u32 pde_start, pde_end, pte_start, pte_end;
    int pde_idx, pte_idx, count;

    pte_start = phys_to_pfn(phys(KERNEL_BASE_VA));
    pte_end = min(maxpfn, phys_to_pfn(DIRECT_MAP_MAX_PA));

    pde_start = KERNEL_BASE_VA >> PAGEDIR_SHIFT;
    pde_end = virt(min(pfn_to_phys(maxpfn), DIRECT_MAP_MAX_PA))
              >> PAGEDIR_SHIFT;

    pte_idx = pte_start;
    for (pde_idx = pde_start; pde_idx <= pde_end; pde_idx++, pde++) {
        pde = (pde_t *)pagedir + pde_idx;
        pte = bootmem_alloc(NR_PTE_PER_PDE * sizeof(pte_t));

        memset((void *)phys(pte), 0, NR_PTE_PER_PDE * sizeof(pte_t));

        set_pde(phys(pde), phys(pte) | PDE_P | PDE_RW | PDE_US);

        count = 0;
        for (; pte_idx <= pte_end && count < NR_PTE_PER_PDE;
               pte_idx++, pte++, count++)
            set_pte(phys(pte), pfn_to_phys(pte_idx) | PTE_P | PTE_RW | PTE_US);
    }

    load_pagetable((pde_t *)phys(pagedir));
}

static void init_pages(void)
{
    int size;

    size = (maxpfn - minpfn + 1) * sizeof(struct page);
    mem_map = bootmem_alloc(size);

    memset(mem_map, 0, size);

    printk("mem_map=%p uses %d bytes, %d pages\n", mem_map, size,
          (size + PAGE_SIZE - 1) >> PAGE_SHIFT);
}

void meminit(void)
{
    get_kernel_data();
    scan_e820map();
    init_bootmem();
    init_mapping();
    init_pages();
    init_buddy();
    free_all_bootmem();

    struct page *page;

    page = alloc_pages(0);
    printk("page=%p, flags=%d, order=%d\n", page, page->flags, page_order(page));
}
