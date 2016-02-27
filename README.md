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
* call `load_kernel` to load the kernel image
  * first we locate the ext2 file system start
  * then we call `loader_ext2_find_file` to locate the image in the file system
  * finally we read the ELF image and load the required segments into memory
  * return the kernel image entry address
* copy these data to `KERNEL_SAVED_DATA` (`0x7e00`) for kernel use
  * `boot_gdt`: GDT set during boot
  * `gdtptr`: pointer to boot GDT
  * `e820map`: e820 memory map obtained in 16-bit C code
* jump to the kernel image entry address


##Kernel entry
This is linked to `.text.entry` section.

The first thing is to set up the mappings for paging.

We'd like to see
* the kernel running in the higher 1 GB of virtual memory
* the lower 3 GB virtual memory are for the user space

We map both regions to the lower 8MB of physical memory
* lower 8 MB of virtual memory
* lower 8 MB of virtual memory starting from the kernel

Then load the base address of the page directory and enable paging.

Finally jump to main function of kernel C code.


##Kernel main function
First initialize the base address of video memory for printing later.

Initialize memory with `meminit()`.


##Memory initialization
First, copy the saved kernel data to kernel memory space, later we'll access memory all through virtual addresses.

Second, `scan_e820map()`, to get the max and min available page frame number (PFN) `maxpfn` and `minpfn`.

Third, `init_bootmem()`, initialize the bootmen allocator.

Fourth, `init_mapping()`, build the new mapping since we do not need the lower 8 MB identity mapping any more,
we access memory only through virtual addresses. The new page directory starts from `pagedir`.
We set up a new mapping, which directly map the lower 896 MB physical memory to the kernel virtual memory space,
which is above `0xc0000000`.

Then, `init_pages()`, allocate a `page` structure for each physical page in the system.

Then, `init_buddy()`



#Memory layout

            +----------------------+
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
       +--- +----------------------+ min(maxpfn, phys_to_pfn(DIRECTMAP_PHYS_MAX))
       |    |                      |
       |    |                      |
       |    |                      |
	   |    |                      |
       |    |                      |
       |    |  managed by bootmem  |
       |    |                      |
       |    +----------------------+ <-- bdata.bitmap + bdata.size
            |   bootmem bitmap     |
    direct  +______________________+ <-- bdata.bitmap
    mapping |                      |
    area    +----------------------+ _end
	        |        ...           | --------------------------------+
	   |    +----------------------+                                 |
	   |    |       pagedir        | 4 KB page directory             |
	   |    +----------------------+                                 |
	   |    |       stack          | 8 KB stack                      |
	   |    +----------------------+                                 +-- .bss
       |    |      pagetable1      | 4 KB page table 1               |
	   |    +----------------------+                                 |
       |    |      pagetable0      | 4 KB page table 2               |
	   |    +----------------------+                                 |
       |    |     init_pagedir     | 4 KB initial page directory     |
	   |    +----------------------+ --------------------------------+
	   |    |       .data          |
	   |    +----------------------+
	   |    |     .rodata          |
	   |    +----------------------+ ---+
	   |    |     .stabstr         |    |
	   |    +----------------------+    +-- for debugging
	   |    |       .stab          |    |
	   |    +----------------------+ ---+
	   |    |       .text          |
	   |    +----------------------+
       |    |     .text.entry      |
       |    +----------------------+ KERNEL_PHYS_START 0x100000, MIN_PA, 1 MB
       |    |     low memory       |
       +--- +----------------------+ 0x0



#Test
##Test if mapping is set up correctly
Read the first byte of each physical page and see if #GP triggers:

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
