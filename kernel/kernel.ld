OUTPUT_FORMAT("elf32-i386")
ENTRY("_start")

KERNEL_START = 0x100000;
KERNEL_VM_START = 0xc0000000;

SECTIONS {
    . = KERNEL_VM_START;

    .text.entry KERNEL_START : AT(KERNEL_START)
    {
        *(.text.entry);
    }

	. = KERNEL_START + KERNEL_VM_START + SIZEOF(.text.entry);

    .text : AT(ADDR(.text) - KERNEL_VM_START)
    {
        _stext = .;
        *(.text);
        _etext = .;
    }

    .data : AT(ADDR(.data) - KERNEL_VM_START)
    {
        _sdata = .;
        *(.rodata .data);
        _edata = .;
    }

    .bss : AT(ADDR(.bss) - KERNEL_VM_START)
    {
        _sbss = .;
        *(.bss);
        _ebss = .;
    }
}
