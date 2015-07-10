#include <>

struct gdt_ptr gdtptr;
struct mem_e820_map e820map;
u64 boot_gdt[N_BOOT_GDT_ENTRY];
