#include <string.h>
#include <asm/x86.h>
#include <asm/e820.h>
#include <asm/mmu.h>
#include <kernel/mm.h>
#include <kernel/types.h>

asm (".code16gcc\n");

struct mem_e820_map e820map;

void print_string(char *s, int len);
void enter_protected_mode(void);

inline static int check_a20(void)
{
    *((u16 *)0x07dfe) = 0xdead; /* write to 0000:7dfe */
    *((u16 *)((0x0ffff << 4) + 0x07e0e)) = 0xbeef; /* write to ffff:7e0e */
    if (*((u16 *)0x07dfe) == 0xbeef) /* check if overwritten */
        return -1;
    return 0;
}

static void wait_read_8042(void)
{
    u8 value;

    while (1) {
        value = inb(0x64);
        if (value & 0x01)
            return;
    }
}

static void wait_write_8042(void)
{
    u8 value;
    
    while (1) {
        value = inb(0x64);
        if (!(value & 0x02))
            return;
    }
}

static int enable_a20(void)
{
    u8 value;

    if (check_a20() == 0)
        return 0;

    /* try BIOS service */
    asm volatile (
            "movw $0x2401, %ax\n\t"
            "int $0x15\n\t"
            );

    if (check_a20() == 0)
        return 0;

    /* try 8042 keyboard controller */
    wait_write_8042();
    outb(0x64, 0xad); /* disable keyboard */
    wait_write_8042();
    outb(0x64, 0xd0);
    wait_read_8042();
    value = inb(0x60);
    wait_write_8042();
    outb(0x64, 0xd1);
    wait_write_8042();
    outb(0x60, value | 0x02); /* enable A20 */
    wait_write_8042();
    outb(0x64, 0xae); /* enable keyboard */
    wait_write_8042();

    if (check_a20() == 0)
        return 0;

    /* try fast A20 gate */
    value = inb(0x92);
    outb(0x92, value | 0x02);
            
    if (check_a20() == 0)
        return 0;
    return -1;
}

inline static void die(void)
{
    print_string("Error occurred during boot\r\n", 28);
    while (1);
}

static int detect_memory(void)
{
    int size = sizeof(struct mem_e820_entry), error;

    asm volatile (
            "xorl %%esi, %%esi\n\t"
            "xorl %%ebx, %%ebx\n\t"
        ".e820_next:\n\t"
            "movl $0x534d4150, %%edx\n\t"
            "movl $0x0000e820, %%eax\n\t"
            "movl $24, %%ecx\n\t"
            "int $0x15\n\t"
            "jc .e820_error\n\t"
            "incl %%esi\n\t"
            "testl %%ebx, %%ebx\n\t"
            "jz .e820_last\n\t"
            "addw %4, %%di\n\t"
            "movl $0x0000e820, %%eax\n\t"
            "movl $24, %%ecx\n\t"
            "jmp .e820_next\n\t"
        ".e820_error:\n\t"
            "movl $0x1, %%eax\n\t"
            "jmp .e820_done\n\t"
        ".e820_last:\n\t"
            "xorl %%eax, %%eax\n\t"
        ".e820_done:\n\t"
            :"=S"(e820map.n_regions), "=a"(error)
            :"S"(e820map.n_regions), "D"(e820map.regions), "m"(size)
            :
            );
    return error ? -1 : 0;
}

u64 boot_gdt[N_BOOT_GDT_ENTRY] = {
    /* 0: null descriptor */
    [0] = seg_desc(0x0, 0x0, 0x0),
    /* 1: 32-bit read/executable code segment, 4k granularity, DPL 0 */
    [BOOT_GDT_ENTRY_CODE] = seg_desc(0x0, 0xfffff, 0xc09a),
    /* 2: 32-bit read/write data segment, 4k granularity, DPL 0 */
    [BOOT_GDT_ENTRY_DATA] = seg_desc(0x0, 0xfffff, 0xc092),
};

struct gdt_ptr boot_gdtptr = {
    .len = sizeof(boot_gdt) - 1,
    .ptr = (u32)&boot_gdt
};

void main(void)
{
    print_string("Enabling A20 line ...\r\n", 23);
    if (enable_a20() < 0)
        die();

    print_string("Detecting memory map ...\r\n", 26);
    if (detect_memory() < 0)
        die();

    print_string("Switching to protected mode ...\r\n", 33);
    enter_protected_mode();
}
