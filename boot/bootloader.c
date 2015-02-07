asm (".code16gcc\n");

#include <arch/x86.h>
#include <include/common.h>

void print_string(char *s, int len);

inline static int check_a20(void)
{
    *((uint16_t *)0x07dfe) = 0xdead; /* write to 0000:7dfe */
    *((uint16_t *)((0x0ffff << 4) + 0x07e0e)) = 0xbeef; /* write to ffff:7e0e */
    if (*((uint16_t *)0x07dfe) == 0xbeef) /* check if overwritten */
        return 0;
    return 1;
}

static void wait_read_8042(void)
{
    uint8_t value;

    while (1) {
        value = inb(0x64);
        if (value & 0x01)
            return;
    }
}

static void wait_write_8042(void)
{
    uint8_t value;
    
    while (1) {
        value = inb(0x64);
        if (!(value & 0x02))
            return;
    }
}

static int enable_a20(void)
{
    uint8_t value;

    if (check_a20())
        return 1;

    /* try BIOS service */
    asm volatile (
            "movw $0x2401, %ax\n\t"
            "int $0x15\n\t"
            );

    if (check_a20())
        return 1;

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

    if (check_a20())
        return 1;

    /* try fast A20 gate */
    value = inb(0x92);
    outb(0x92, value | 0x02);
            
    if (check_a20())
        return 1;
    return 0;
}

inline static void print_status(int status)
{
    if (status == 1)
        print_string("done\r\n", 6);
    else
        print_string("failed\r\n", 8);
}

struct mem_e820_entry {
    uint64_t base; 
    uint64_t len;
    uint32_t type;
    uint32_t attr;
};

struct mem_e820 {
    uint32_t n_regions;
    struct mem_e820_entry regions[MEM_E820_MAX];
} e820_map;

static int detect_memory(void)
{
    int size = sizeof(struct mem_e820_entry), ret;

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
            "xorl %%eax, %%eax\n\t"
            "jmp .e820_done\n\t"
        ".e820_last:\n\t"
            "movl $0x1, %%eax\n\t"
        ".e820_done:\n\t"
            :"=S"(e820_map.n_regions), "=a"(ret)
            :"S"(e820_map.n_regions), "D"(e820_map.regions), "m"(size)
            :
            );
    return ret;
}

#define PRINT_STATUS(function_call)        \
    do {                                   \
        if (function_call)                 \
            print_string("done\r\n", 6);   \
        else                               \
            print_string("failed\r\n", 8); \
    } while (0)

void main(void)
{
    print_string("Enabling A20 line ...", 21);
    PRINT_STATUS(enable_a20());

    print_string("Detecting memory map ...", 24);
    PRINT_STATUS(detect_memory());

    while (1);
}
