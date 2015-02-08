SECTIONS {
    . = 0x500;
    .text : {
        *(.text*);
    }
    .data : {
        *(.rodata .data);
    }
    .bss : {
        *(.bss);
    }
}
