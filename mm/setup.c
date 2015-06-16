#include <string.h>
#include <asm/e820.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/kerndata.h>
#include <kernel/mm.h>

struct gdt_ptr gdtptr;
struct mem_e820_map e820map;

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

void scan_e820map(void)
{
    int i;

    for (i = 0; i < e820map.n_regions; i++) {
        printk("base=%016X  len=%016X  type=%d\n",
                e820map.regions[i].base,
                e820map.regions[i].len,
                e820map.regions[i].type);
    }
}

void meminit(void)
{
    get_kernel_data();
    scan_e820map();
}
