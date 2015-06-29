OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY("_start")

KERNEL_PHYS_START = 0x100000;
KERNEL_VIRT_START = 0xc0000000;

SECTIONS {
    . = KERNEL_PHYS_START;

    .text.entry : AT(KERNEL_PHYS_START)
    {
        PROVIDE(_stextentry = .);
        *(.text.entry);
        PROVIDE(_etextentry = .);
    }

    . = KERNEL_PHYS_START + KERNEL_VIRT_START + SIZEOF(.text.entry);

    .text : AT(ADDR(.text) - KERNEL_VIRT_START)
    {
        PROVIDE(_stext = .);
        *(.text .stab);
        PROVIDE(_etext = .);
    }

    /* Include debugging information in kernel memory */
    .stab : AT(ADDR(.stab) - KERNEL_VIRT_START)
    {
        PROVIDE(_sstab = .);
        *(.stab);
        PROVIDE(_estab = .);
        BYTE(0)
    }

    .stabstr : AT(ADDR(.stabstr) - KERNEL_VIRT_START)
    {
        PROVIDE(_sstabstr = .);
        *(.stabstr);
        PROVIDE(_estabstr = .);
        BYTE(0)
    }

	.rodata : AT(ADDR(.rodata) - KERNEL_VIRT_START)
	{
        PROVIDE(_srodata = .);
        *(.rodata .rodata.*);
        PROVIDE(_erodata = .);
	}

    .data : AT(ADDR(.data) - KERNEL_VIRT_START)
    {
        PROVIDE(_sdata = .);
        *(.data);
        PROVIDE(_edata = .);
    }

    .bss : AT(ADDR(.bss) - KERNEL_VIRT_START)
    {
        PROVIDE(_sbss = .);
        *(.bss);
        PROVIDE(_ebss = .);
    }

    PROVIDE(_end = .);
}
