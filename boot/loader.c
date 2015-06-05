#include <string.h>
#include <elf.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <kernel/ide.h>
#include <kernel/console.h>
#include <kernel/ext2.h>
#include <kernel/mm.h>

#define BUFSZ 1024

static int load_kernel(void) {
    int i, ret, sz, loadsz;
    uint8_t *addr, buf[BUFSZ];
    uint32_t phdr_off, phyaddr;
    struct ext2_fsinfo fs;
    struct ext2_inode ino;
    struct Elf32_Ehdr elf;
    struct Elf32_Phdr phdr;

    /* get LBA of first sector from partition table */
    addr = (uint8_t *)0x7dbe; /* 0x7c00 + 446 */
    while (*addr != 0x80 || *(addr + 4) != 0x83)
        addr += 0x10;
    fs.disk_start = (*((uint32_t *)(addr + 8))) << 9;

    loader_ext2_get_fsinfo(&fs);

    if ((ret = loader_ext2_find_file(&fs, "/boot/kernel.img", &ino)) == -1)
        return -1;
    if (ret == 0) {
        printk("Cannot find kernel image\n");
        return 0;
    }

    printk("Loading ELF...\n");
    loader_ext2_read(&fs, &ino, &elf, sizeof(struct Elf32_Ehdr));
    if (elf.e_ident[EI_MAG0] != 0x7f
            || elf.e_ident[EI_MAG1] != 'E'
            || elf.e_ident[EI_MAG2] != 'L'
            || elf.e_ident[EI_MAG3] != 'F') {
        printk("Bad ELF file\n");
        return -1;
    }

    phdr_off = elf.e_phoff;
    for (i = 0; i < elf.e_phnum; i++) {
        if (loader_ext2_pread(&fs, &ino, &phdr, elf.e_phentsize, phdr_off) == -1)
            return -1;
        if (phdr.p_type != PT_LOAD)
            continue;

        phyaddr = phdr.p_vaddr >= KERNEL_VM_START ?
            phdr.p_vaddr - KERNEL_VM_START : phdr.p_vaddr;
        loadsz = 0;

        while (loadsz < phdr.p_filesz) {
            sz = BUFSZ < (phdr.p_filesz - loadsz) ?
                 BUFSZ : (phdr.p_filesz - loadsz);

            if (loader_ext2_pread(&fs, &ino, buf, sz, phdr.p_offset + loadsz) == -1)
                return -1;

            memcpy((char *)(phyaddr + loadsz), buf, sz);

            loadsz += sz;
        }

        if (loadsz < phdr.p_memsz)
            memset((char *)(phyaddr + loadsz), 0, phdr.p_memsz - loadsz);

        phdr_off += elf.e_phentsize;
    }

    ((void(*)(void))elf.e_entry)();

    return 0;
}

void pm_main(void)
{
    cons_clear_screen();

    ata_init();

    if (load_kernel()) {
        printk("Failed to load kernel\n");
        while (1);
    }

    while (1);
}
