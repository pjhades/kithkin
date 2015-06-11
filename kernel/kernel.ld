OUTPUT_FORMAT("elf32-i386")
ENTRY("_start")

KERNEL_START = 0x100000;
KERNEL_VM_START = 0xc0000000;

SECTIONS {
    .text.entry KERNEL_START : AT(KERNEL_START)
    {
        *(.text.entry);
    }

	. = KERNEL_START + KERNEL_VM_START + SIZEOF(.text.entry);

    PROVIDE(_stext = .);
    .text : AT(ADDR(.text) - KERNEL_VM_START)
    {
        *(.text);
    }
    PROVIDE(_etext = .);

    PROVIDE(_sdata = .);
    .data : AT(ADDR(.data) - KERNEL_VM_START)
    {
        *(.rodata .data);
    }
    PROVIDE(_edata = .);

    PROVIDE(_sbss = .);
    .bss : AT(ADDR(.bss) - KERNEL_VM_START)
    {
        *(.bss);
    }
    PROVIDE(_ebss = .);
}
