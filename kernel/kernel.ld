OUTPUT_FORMAT("elf32-i386")
ENTRY("_start")

KERNEL_START = 0x100000;
KERNEL_VM_START = 0xc0000000;

SECTIONS {
    . = KERNEL_START;

    .text.entry : AT(KERNEL_START)
    {
        PROVIDE(_stextentry = .);
        *(.text.entry);
        PROVIDE(_etextentry = .);
    }

	. = KERNEL_START + KERNEL_VM_START + SIZEOF(.text.entry);

    .text : AT(ADDR(.text) - KERNEL_VM_START)
    {
        PROVIDE(_stext = .);
        *(.text);
        PROVIDE(_etext = .);
    }

    .data : AT(ADDR(.data) - KERNEL_VM_START)
    {
        PROVIDE(_sdata = .);
        *(.rodata .data);
        PROVIDE(_edata = .);
    }

    .bss : AT(ADDR(.bss) - KERNEL_VM_START)
    {
        PROVIDE(_sbss = .);
        *(.bss);
        PROVIDE(_ebss = .);
    }

    PROVIDE(_end = .);
}
