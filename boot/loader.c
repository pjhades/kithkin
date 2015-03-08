#include <elf.h>
#include <kernel/types.h>
#include <kernel/ide.h>
#include <kernel/console.h>
#include <kernel/ext2.h>

struct console vga;

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
    if (elf.e_ident[EI_MAG0] != 0x7f
            || elf.e_ident[EI_MAG1] != 'E'
            || elf.e_ident[EI_MAG2] != 'L'
            || elf.e_ident[EI_MAG3] != 'F') {
        cons_puts("Bad ELF file\n");
        return -1;
    }

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

    ((void(*)(void))elf.e_entry)();

    return 0;
}

void pm_main(void)
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
