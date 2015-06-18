#include <string.h>
#include <asm/e820.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/kerndata.h>
#include <kernel/mm.h>

struct gdt_ptr gdtptr;
struct mem_e820_map e820map;

/* page frame number of the minimal/maximal usable physical page */
uint32_t minpfn, maxpfn;

static void get_kernel_data(void)
{
    char *dst = (char *)KERNEL_STARTUP_DATA;
    unsigned char type;
    uint32_t size;

    type = *((unsigned char *)dst);
    while (type != KERNDATA_NONE) {
        dst += sizeof(unsigned char);
        size = *((uint32_t *)dst);
        dst += sizeof(uint32_t);
        if (type == KERNDATA_BOOTGDT) {
            //TODO boot_gdt[] is not copied
            dst += sizeof(uint64_t) * N_BOOT_GDT_ENTRY;
        }
        else if (type == KERNDATA_BOOTGDTPTR) {
            memcpy(&gdtptr, dst, size);
            dst += sizeof(struct gdt_ptr);
        }
        else if (type == KERNDATA_E820) {
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
    uint32_t end;

    minpfn = 0xffffffff;
    maxpfn = 0;

    for (i = 0; i < e820map.n_regions; i++) {
        printk("e820: range 0x%016X - 0x%016X, %s\n", e820map.regions[i].base,
                e820map.regions[i].base + e820map.regions[i].len - 1,
                e820map.regions[i].type == E820_USABLE ? "usable": "reserved");

        if (e820map.regions[i].type != E820_USABLE)
            continue;
        end = e820map.regions[i].base + e820map.regions[i].len - 1;
        if (e820map.regions[i].base >= MIN_PHYS
                && e820map.regions[i].base < minpfn)
            minpfn = e820map.regions[i].base;
        if (end > maxpfn)
            maxpfn = end;
    }
    if (minpfn == 0xffffffff)
        die("e820: no minimal usable memory above %x\n", MIN_PHYS);
    minpfn = phys_to_pfn(p2roundup(minpfn, PAGE_SIZE));
    maxpfn = phys_to_pfn(p2rounddown(maxpfn, PAGE_SIZE));
    if (minpfn > maxpfn)
        die("e820: no usable range found\n");
    printk("e820: minpfn = %p, maxpfn = %p\n", minpfn, maxpfn);
}

void meminit(void)
{
    get_kernel_data();
    scan_e820map();
}
