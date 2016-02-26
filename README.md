#Booting
##Boot sector
Boot sector is implemented in `setctor.S`, check partition to find the first bootable linux partition.
Then call `int 0x13` to load the bootloader from `C:H:S = 0:0:2`. Finally
jump to bootloader.

##Bootloader
Bootloader passes control to the kernel. We do this in a C-assembly-C manner.

First run `main` from `loader_16bit.c`. In this file we
* enable A20 line to access memory higher than 1M
* call `int 0x15` to detect memory map
* set up GDT
  * one 32-bit read/executable code segment, 4K granularity, DPL 0
  * one 32-bit read/write data segment, 4K granularity, DPL 0
* call `enter_protected_mode`

Then in `loader.S`, we
* enter the protected mode
* call `loader_main` to the loader code

Finally in `loader.c`, we
* clear the text-based console
* initialize the IDE device
* call `load_kernel`
  * first we locate the ext2 file system start
  * then we call `loader_ext2_find_file` to locate the image in the file system
  * finally we read the ELF image and load the required segments into memory
  * return the kernel image entry address
* jump to the kernel image entry address


#Physical Memory

            +----------------------+
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
       +--- +----------------------+ min(maxpfn, phys_to_pfn(DIRECTMAP_PHYS_MAX))
       |    |                      |
       |    |                      |
       |    |                      | <-- end of hardcoded 8MB initial
	   |    |                      |      mapping in kernel/entry.S
       |    |                      |
       |    |  managed by bootmem  |
       |    |                      |
       |    +----------------------+ <-- pfn_up((pfn_up(_end) << PAGE_SHIFT) + bdata.size);
            |                      |      make enough room for the bitmap
    direct  |   bootmem bitmap     |
            +______________________+ <-- pfn_up(_end) aligned by page
    mapping |                      |
    area    +----------------------+ _end
            |                      |
       |    |                      | <-- initial mapping page tables
       |    |                      |        in .bss section
       |    |   kernel image       |
       |    |                      |
       |    +----------------------+ KERNEL_PHYS_START 0x100000
       |    |                      |
       |    |                      |
       +--- +----------------------+ 0



#Test
##Test if mapping is set up correctly
Read the first byte of each physical page and see if error triggers:

```C
    char byte;
    for (pfn = minpfn; pfn <= pte_max; pfn++) {
        byte = *((char *)(pfn << PAGE_SHIFT));
        byte += 0; /* eliminate compiler warning */
        printk("physical page %d/%d ok\n", pfn, pte_max);
    }

    /* bad: not mapped page */
    byte = *((char *)((pte_max + 1) << PAGE_SHIFT));

    /* ok: last byte of the last mapped page */
    byte = *((char *)((pte_max + 1) << PAGE_SHIFT) - 1);
```
