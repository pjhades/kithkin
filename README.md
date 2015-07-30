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
