#include <string.h>
#include <asm/e820.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/bootdata.h>
#include <kernel/mm.h>
#include <kernel/alloc.h>

struct gdt_ptr gdtptr;
struct mem_e820_map e820map;

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
            //TODO boot_gdt[] is not copied
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

//static void *raw_alloc(uint32_t size)
//{
//}

static void init_memmap(void)
{
    extern char pagedir[];
    pde_t *pde;
    pte_t *pte;
    uint32_t pfn, end, pde_max, pte_max;
    int pde_idx;

    pde = (pde_t *)pagedir;
    pte = (pte_t *)(pde + N_PDE * sizeof(pde_t));
    pfn = 0;

    memset((void *)phys(pagedir), 0, sizeof(pde_t) * N_PDE);

    /* maximum directly mapped PDE and PTE index */
    pte_max = min(maxpfn, phys_to_pfn(DIRECTMAP_PHYS_MAX));
    pde_max = pte_max >> (PAGEDIR_SHIFT - PAGE_SHIFT);

    for (pde_idx = 0; pde_idx <= pde_max; pde_idx++, pde++) {
        set_pde(pde, phys(pte) | PDE_P | PDE_RW | PDE_US);

        memset((void *)phys(pte), 0, sizeof(pte_t) * N_PTE_PER_PDE);

        end = (pde_idx + 1) * N_PTE_PER_PDE - 1;
        for (; pfn <= min(pte_max, end); pfn++, pte++)
            set_pte(pte, (pfn << PAGE_SHIFT) | PTE_P | PTE_RW | PTE_US);
    }

    memcpy((void *)phys(pagedir) + sizeof(pde_t) * N_USER_PDE,
           (void *)phys(pagedir), sizeof(pde_t) * pde_max);
    load_pagetable((pde_t *)phys(pagedir));
}

void meminit(void)
{
    get_kernel_data();
    scan_e820map();
    init_bootmem();
    init_memmap();
}
