#include <common.h>
#include <arch/x86.h>
#include <arch/mmu.h>
#include <driver/ide.h>
#include <fs/ext2.h>

asm (".code16gcc\n");

uint8_t cursor_row, cursor_col;

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

static void save_cursor_position(void)
{
    asm volatile(
            "movb $0x03, %%ah\n\t"
            "xorb %%bh, %%bh\n\t"
            "int $0x10\n\t"
            "movb %%dh, %%al\n\t"
            :"=a"(cursor_row), "=d"(cursor_col)
            );
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
    save_cursor_position(); /* to continue printing */
    enter_protected_mode();
}


/* run in protected mode */
asm (".code32\n");

// TODO replace this with console functions
static void pm_printc(char ch)
{
    if (ch == ' ') {
        if (++cursor_col == 80)
            goto newline;
        return;
    }
    if (ch == '\n') {
newline:
        ++cursor_row;
        cursor_col = 0;
        return;
    }
    if (ch == '\r') {
        cursor_col = 0;
        return;
    }
    *((uint8_t *)0x0b8000 + ((cursor_row * 80 + cursor_col) << 1)) = ch;
    *((uint8_t *)0x0b8001 + ((cursor_row * 80 + cursor_col) << 1)) = 0x0e;
    if (++cursor_col == 80) {
        cursor_col = 0;
        ++cursor_row;
    }
}

// TODO replace this with console functions
static void pm_print(char *s, int len)
{
    for (; len; s++, len--)
        pm_printc(*s);
}

// TODO replace this with console functions
static void pm_printhex(uint32_t hex, int unit)
{
    /* unit = 1, 2, 4 */
    char *table = "0123456789abcdef";
    uint32_t v32;
    uint16_t v16;
    uint8_t v8;

    if (unit == 1) {
        if (hex == 0)
            pm_print("00 ", 3);
        else {
            v32 = hex;
            while (v32) {
                v8 = v32 % 256;
                pm_printc(table[(v8 & 0xf0) >> 4]);
                pm_printc(table[v8 & 0x0f]);
                pm_printc(' ');
                v32 >>= 8;
            }
        }
    }
    else if (unit == 2) {
        if (hex == 0)
            pm_print("0000 ", 5);
        else {
            v32 = hex;
            while (v32) {
                v16 = v32 % 65536;
                pm_printc(table[(v16 & 0xf000) >> 12]);
                pm_printc(table[(v16 & 0x0f00) >> 8]);
                pm_printc(table[(v16 & 0x00f0) >> 4]);
                pm_printc(table[v16 & 0x0f]);
                pm_printc(' ');
                v32 >>= 16;
            }
        }
    }
    else {
        if (hex == 0)
            pm_print("00000000 ", 9);
        else {
            v32 = hex;
            pm_printc(table[(v32 & 0xf0000000) >> 28]);
            pm_printc(table[(v32 & 0x0f000000) >> 24]);
            pm_printc(table[(v32 & 0x00f00000) >> 20]);
            pm_printc(table[(v32 & 0x000f0000) >> 16]);
            pm_printc(table[(v32 & 0x0000f000) >> 12]);
            pm_printc(table[(v32 & 0x00000f00) >> 8]);
            pm_printc(table[(v32 & 0x000000f0) >> 4]);
            pm_printc(table[v32 & 0x0f]);
            pm_printc(' ');
        }
    }
}

static int load_kernel(void) {
    uint8_t *addr, block[4096];
    uint32_t n_blkgrps, grp_id, blk_size, n_desc_blks, blk_id;
    uint64_t offset;
    struct ext2_superblock sb;
    struct ext2_block_group_desc *desc_table;

    addr = (uint8_t *)0x7dbe; /* 0x7c00 + 446 */
    while (*addr != 0x80 || *(addr + 4) != 0x83)
        addr += 0x10;
    offset = (*((uint32_t *)(addr + 8))) << 9;

    /* Read superblock */
    if (ide_read(PTR2LBA(offset + 1024), 2, (uint8_t *)&sb))
        return -1;

    /* Read group descriptor table */
    offset += 2048;
    blk_size = 1024 << sb.sb_log_block_size;
    n_blkgrps = (sb.sb_n_blocks + sb.sb_n_blocks_per_blkgrp - 1)
        / sb.sb_n_blocks_per_blkgrp;
    grp_id = (EXT2_ROOT_INODE - 1) / sb.sb_n_inodes_per_blkgrp;
    n_desc_blks = (sizeof(struct ext2_block_group_desc) * n_blkgrps
            + blk_size - 1) / blk_size;
    blk_id = grp_id / n_desc_blks;

    if (ext2_read_block(&sb, offset, block))
        return -1;

    desc_table = (struct ext2_block_group_desc *)block;
    pm_printhex(desc_table[grp_id].bg_block_bitmap, 4);
    pm_printc('\n');
    pm_printhex(desc_table[grp_id].bg_inode_bitmap, 4);
    pm_printc('\n');
    pm_printhex(desc_table[grp_id].bg_inode_table, 4);
    pm_printc('\n');
    pm_printhex(desc_table[grp_id].bg_n_free_blocks, 2);
    pm_printc('\n');
    pm_printhex(desc_table[grp_id].bg_n_free_inodes, 2);
    pm_printc('\n');
    pm_printhex(desc_table[grp_id].bg_n_dirs, 2);
    pm_printc('\n');

    return 0;
}

void pm_main()
{
    uint8_t id1, id2;
    struct ide_dev drv;

    ide_init();
    if (ide_read_identity(&drv.iden)) {
        pm_print("Failed to read IDE identity", 27);
        while (1);
    }

    if (load_kernel()) {
        pm_print("Failed to read IDE identity", 27);
        while (1);
    }

    while (1);
}
