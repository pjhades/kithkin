#include <string.h>
#include <asm/e820.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/bootdata.h>
#include <kernel/mm.h>
#include <kernel/alloc.h>

struct gdt_ptr gdtptr;
struct mem_e820_map e820map;
uint64_t boot_gdt[N_BOOT_GDT_ENTRY];

struct page *mem_map;

/* page frame number of the minimal/maximal usable physical page */
uint32_t minpfn, maxpfn;

static void get_kernel_data(void)
{
    char *dst = (char *)KERNEL_BOOTDATA;
    unsigned char type;
    uint32_t size;

    type = *((unsigned char *)dst);
    while (type != BOOTDATA_NONE) {
        dst += sizeof(unsigned char);
        size = *((uint32_t *)dst);
        dst += sizeof(uint32_t);

        if (type == BOOTDATA_BOOTGDT) {
            memcpy(boot_gdt, dst, size);
            dst += sizeof(uint64_t) * N_BOOT_GDT_ENTRY;
        }
        else if (type == BOOTDATA_BOOTGDTPTR) {
            memcpy(&gdtptr, dst, size);
            dst += sizeof(struct gdt_ptr);
        }
        else if (type == BOOTDATA_E820) {
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
    uint32_t maxsize;

    minpfn = 0xffffffff;
    maxpfn = 0;
    maxsize = 0;

    for (i = 0; i < e820map.n_regions; i++) {
        printk("e820: range 0x%016X - 0x%016X, %s\n", e820map.regions[i].base,
                e820map.regions[i].base + e820map.regions[i].len - 1,
                e820map.regions[i].type == E820_USABLE ? "usable": "reserved");

        if (e820map.regions[i].type != E820_USABLE)
            continue;
        if (e820map.regions[i].len < maxsize
                || e820map.regions[i].base < MIN_PHYS)
            continue;
        minpfn = e820map.regions[i].base;
        maxpfn = e820map.regions[i].base + e820map.regions[i].len - 1;
        maxsize = e820map.regions[i].len;
    }

    if (maxsize == 0)
        die("e820: no minimal usable memory above %x\n", MIN_PHYS);

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

static void init_memmap(void)
{
    extern char pagedir[];
    pde_t *pde;
    pte_t *pte;
    uint32_t pde_start, pde_end, pte_start, pte_end;
    int pde_idx, pte_idx, count;

    pte_start = phys_to_pfn(phys(KERNEL_VIRT_START));
    pte_end = min(maxpfn, phys_to_pfn(DIRECTMAP_PHYS_MAX));

    pde_start = KERNEL_VIRT_START >> PAGEDIR_SHIFT;
    pde_end = virt(min(pfn_to_phys(maxpfn), DIRECTMAP_PHYS_MAX))
              >> PAGEDIR_SHIFT;

    pte_idx = pte_start;
    for (pde_idx = pde_start; pde_idx <= pde_end; pde_idx++, pde++) {
        pde = (pde_t *)pagedir + pde_idx;
        pte = bootmem_alloc(N_PTE_PER_PDE * sizeof(pte_t));

        memset((void *)phys(pte), 0, N_PTE_PER_PDE * sizeof(pte_t));

        set_pde(phys(pde), phys(pte) | PDE_P | PDE_RW | PDE_US);

        count = 0;
        for (; pte_idx <= pte_end && count < N_PTE_PER_PDE;
               pte_idx++, pte++, count++)
            set_pte(phys(pte), pfn_to_phys(pte_idx) | PTE_P | PTE_RW | PTE_US);
    }

    load_pagetable((pde_t *)phys(pagedir));
}

void meminit(void)
{
    get_kernel_data();
    scan_e820map();
    init_bootmem();
    init_memmap();
}
