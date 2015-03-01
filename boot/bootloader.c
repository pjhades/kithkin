#include <common.h>
#include <elf.h>
#include <arch/x86.h>
#include <arch/mmu.h>
#include <driver/ide.h>
#include <driver/tty.h>
#include <fs/ext2.h>

asm (".code16gcc\n");

struct console vga;

void print_string(char *s, int len);

inline static int check_a20(void)
{
    *((uint16_t *)0x07dfe) = 0xdead; /* write to 0000:7dfe */
    *((uint16_t *)((0x0ffff << 4) + 0x07e0e)) = 0xbeef; /* write to ffff:7e0e */
    if (*((uint16_t *)0x07dfe) == 0xbeef) /* check if overwritten */
        return -1;
    return 0;
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

    if (!check_a20())
        return 0;

    /* try BIOS service */
    asm volatile (
            "movw $0x2401, %ax\n\t"
            "int $0x15\n\t"
            );

    if (!check_a20())
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

    if (!check_a20())
        return 0;

    /* try fast A20 gate */
    value = inb(0x92);
    outb(0x92, value | 0x02);
            
    if (!check_a20())
        return 0;
    return -1;
}

inline static void die(void)
{
    print_string("Error occurred during boot\r\n", 28);
    while (1);
}

struct mem_e820_entry {
    uint64_t base; 
    uint64_t len;
    uint32_t type;
    uint32_t attr;
};

#define MEM_E820_MAX 128
struct mem_e820 {
    uint32_t n_regions;
    struct mem_e820_entry regions[MEM_E820_MAX];
} e820_map;

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
            :"=S"(e820_map.n_regions), "=a"(error)
            :"S"(e820_map.n_regions), "D"(e820_map.regions), "m"(size)
            :
            );
    return error ? -1 : 0;
}

uint64_t boot_gdt[] = {
    /* 0: null descriptor */
    [0] = SEG_DESC(0x0, 0x0, 0x0),
    /* 1: 32-bit read/executable code segment, 4k granularity, DPL 0 */
    [BOOT_GDT_ENTRY_CODE] = SEG_DESC(0x0, 0xfffff, 0xc09a),
    /* 2: 32-bit read/write data segment, 4k granularity, DPL 0*/
    [BOOT_GDT_ENTRY_DATA] = SEG_DESC(0x0, 0xfffff, 0xc092),
};

struct gdt_ptr {
    uint16_t len;
    uint32_t ptr;
} __attribute__((packed));

struct gdt_ptr gdt;

static void enter_protected_mode(void)
{
    gdt.len = sizeof(boot_gdt) - 1;
    gdt.ptr = (uint32_t)&boot_gdt;
    asm volatile (
            "lgdt %0\n\t"
            "movl %%cr0, %%eax\n\t"
            "orl $0x1, %%eax\n\t"
            "movl %%eax, %%cr0\n\t"
            : :"m"(gdt) :
            );
    jump_to_protected_mode();
}

void main(void)
{
    print_string("Enabling A20 line ...\r\n", 23);
    if (enable_a20())
        die();

    print_string("Detecting memory map ...\r\n", 26);
    if (detect_memory())
        die();

    print_string("Switching to protected mode ...\r\n", 33);
    enter_protected_mode();
}

/*
 * ----------------------------------------------
 *           run in protected mode 
 * ----------------------------------------------
 */
asm (".code32\n");

static int load_kernel(void) {
    int i, ret, sz, loadsz;
#define BUFSZ 1024
    uint8_t *addr, buf[BUFSZ];
    struct ext2_fsinfo fs;
    struct ext2_inode ino;
    struct Elf32_Ehdr elf;
    struct Elf32_Phdr phdr;

    /* get LBA of first sector from partition table */
    addr = (uint8_t *)0x7dbe; /* 0x7c00 + 446 */
    while (*addr != 0x80 || *(addr + 4) != 0x83)
        addr += 0x10;
    fs.disk_start = (*((uint32_t *)(addr + 8))) << 9;

    boot_ext2_get_fsinfo(&fs);

    if ((ret = boot_ext2_find_file(&fs, "/boot/kernel.img", &ino)) == -1)
        return -1;
    if (ret == 0) {
        cons_puts("Cannot find kernel image\n");
        return 0;
    }

    cons_puts("Loading ELF...\n");
    boot_ext2_read(&fs, &ino, &elf, sizeof(struct Elf32_Ehdr));

    cons_puthex(elf.e_ident[0]);
    cons_putchar('\n');
    cons_putchar(elf.e_ident[1]);
    cons_putchar(elf.e_ident[2]);
    cons_putchar(elf.e_ident[3]);
    cons_putchar('\n');

    for (i = 0; i < elf.e_phnum; i++) {
        if (boot_ext2_pread(&fs, &ino, &phdr, elf.e_phentsize,
                    elf.e_phoff) == -1)
            return -1;
        if (phdr.p_type != PT_LOAD)
            continue;
        loadsz = 0;
        while (loadsz < phdr.p_filesz) {
            sz = BUFSZ < phdr.p_filesz ? BUFSZ : phdr.p_filesz;
            if (boot_ext2_pread(&fs, &ino, buf, sz, phdr.p_offset) == -1)
                return -1;
            memcpy(phdr.p_vaddr + loadsz, buf, sz);
            loadsz += sz;
        }
    }

    asm volatile (
            "jmp %%eax\n\t"
            :
            :"a"(phdr.p_vaddr)
            :
            );

    return 0;
}

void pm_main()
{
    uint8_t id1, id2;
    struct ide_dev drv;

    cons_clear_screen();

    ide_init();
    if (ide_read_identity(&drv.iden)) {
        cons_puts("Failed to read IDE identity\n");
        while (1);
    }

    if (load_kernel()) {
        cons_puts("Failed to load kernel\n");
        while (1);
    }

    while (1);
}
