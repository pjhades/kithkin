#Physical Memory

            +-----------------------------+
            |                             |
            |                             |
            |                             |
            |                             |
            |                             |
            |                             |
       +--- +-----------------------------+ min(maxpfn, phys_to_pfn(DIRECTMAP_PHYS_MAX))
       |    |                             |
       |    |                             |
       |    |                             | <----- end of hardcoded initial
       |    |     managed by easyalloc    |        mapping in kernel/entry.S
       |    |                             |
            +-----------------------------+
    direct  |      easyalloc bitmap       |
    mapping |                             |
    area    +-----------------------------+ _end
            |                             |
       |    |                             | <----- initial mapping page tables
       |    |                             |        in .bss section
       |    |      kernel image           |
       |    |                             |
       |    +-----------------------------+ KERNEL_PHYS_START 0x100000
       |    |                             |
       |    |                             |
       +--- +-----------------------------+ 0

