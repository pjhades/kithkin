#include <string.h>
#include <elf.h>
#include <ide.h>
#include <console.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/ext2.h>
#include <kernel/bootdata.h>
#include <kernel/mm.h>

#define BUFSZ 1024

static void *load_kernel(void)
{
    int i, ret, sz, loadsz;
    u8 *addr, buf[BUFSZ];
    u32 phdr_off, phyaddr;
    struct ext2_fsinfo fs;
    struct ext2_inode ino;
    struct elf32_ehdr elf;
    struct elf32_phdr phdr;

    /* get LBA of first sector from partition table */
    addr = (u8 *)0x7dbe; /* 0x7c00 + 446 */
    while (*addr != 0x80 || *(addr + 4) != 0x83)
        addr += 0x10;
    fs.disk_start = (*((u32 *)(addr + 8))) << 9;

    loader_ext2_get_fsinfo(&fs);
    if ((ret = loader_ext2_find_file(&fs, "/boot/kernel.img", &ino)) == -1)
        return NULL;
    if (ret == 0) {
        printk("Cannot find kernel image\n");
        return NULL;
    }

    printk("Loading ELF...\n");
    loader_ext2_read(&fs, &ino, &elf, sizeof(struct elf32_ehdr));
    if (elf.e_ident[EI_MAG0] != 0x7f
            || elf.e_ident[EI_MAG1] != 'E'
            || elf.e_ident[EI_MAG2] != 'L'
            || elf.e_ident[EI_MAG3] != 'F') {
        printk("Bad ELF file\n");
        return NULL;
    }

    phdr_off = elf.e_phoff;
    for (i = 0; i < elf.e_phnum; i++) {
        if (loader_ext2_pread(&fs, &ino, &phdr, elf.e_phentsize, phdr_off) == -1)
            return NULL;
        if (phdr.p_type != PT_LOAD)
            continue;

        if (phdr.p_vaddr >= KERNEL_VIRT_START)
            phyaddr = phdr.p_vaddr - KERNEL_VIRT_START;
        else
            phyaddr = phdr.p_vaddr;

        loadsz = 0;
        while (loadsz < phdr.p_filesz) {
            sz = BUFSZ < (phdr.p_filesz - loadsz) ?
                 BUFSZ : (phdr.p_filesz - loadsz);
            if (loader_ext2_pread(&fs, &ino, buf, sz, phdr.p_offset + loadsz) == -1)
                return NULL;
            memcpy((char *)(phyaddr + loadsz), buf, sz);
            loadsz += sz;
        }
        if (loadsz < phdr.p_memsz)
            memset((char *)(phyaddr + loadsz), 0, phdr.p_memsz - loadsz);
        phdr_off += elf.e_phentsize;
    }

    return (void *)elf.e_entry;
}

static void copy_kernel_data(void)
{
    char *dst = (char *)KERNEL_BOOTDATA;
#define COPY(ptr, var, tag) \
    do { \
        *((unsigned char *)(ptr)) = tag; \
        (ptr) += sizeof(unsigned char); \
        *((u32 *)(ptr)) = sizeof((var)); \
        (ptr) += sizeof(u32); \
        memcpy((ptr), (void *)&(var), sizeof((var))); \
        (ptr) += sizeof((var)); \
    } while (0)

    COPY(dst, boot_gdt,    BOOTDATA_BOOTGDT);
    COPY(dst, boot_gdtptr, BOOTDATA_BOOTGDTPTR);
    COPY(dst, e820map,     BOOTDATA_E820);
#undef COPY
    *((unsigned char *)dst) = BOOTDATA_NONE;
}

struct console_device console;

void loader_main(void)
{
    void *entry;

    console_init(CONSOLE_MEM_DATA);
    ata_init();
    if (!(entry = load_kernel())) {
        printk("Failed to load kernel\n");
        while (1);
    }
    copy_kernel_data();
    ((void (*)(void))entry)();
    while (1);
}
