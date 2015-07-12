OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY("_start")

KERNEL_BASE_PA = 0x100000;
KERNEL_BASE_VA = 0xc0000000;

SECTIONS {
    . = KERNEL_BASE_PA;

    .text.entry : AT(KERNEL_BASE_PA)
    {
        PROVIDE(_stextentry = .);
        *(.text.entry);
        PROVIDE(_etextentry = .);
    }

    . = KERNEL_BASE_PA + KERNEL_BASE_VA + SIZEOF(.text.entry);

    .text : AT(ADDR(.text) - KERNEL_BASE_VA)
    {
        PROVIDE(_stext = .);
        *(.text .stab);
        PROVIDE(_etext = .);
    }

    /* Include debugging information in kernel memory */
    .stab : AT(ADDR(.stab) - KERNEL_BASE_VA)
    {
        PROVIDE(_sstab = .);
        *(.stab);
        PROVIDE(_estab = .);
        BYTE(0)
    }

    .stabstr : AT(ADDR(.stabstr) - KERNEL_BASE_VA)
    {
        PROVIDE(_sstabstr = .);
        *(.stabstr);
        PROVIDE(_estabstr = .);
        BYTE(0)
    }

	.rodata : AT(ADDR(.rodata) - KERNEL_BASE_VA)
	{
        PROVIDE(_srodata = .);
        *(.rodata .rodata.*);
        PROVIDE(_erodata = .);
	}

    .data : AT(ADDR(.data) - KERNEL_BASE_VA)
    {
        PROVIDE(_sdata = .);
        *(.data);
        PROVIDE(_edata = .);
    }

    .bss : AT(ADDR(.bss) - KERNEL_BASE_VA)
    {
        PROVIDE(_sbss = .);
        *(.bss);
        PROVIDE(_ebss = .);
    }

    PROVIDE(_end = .);
}
