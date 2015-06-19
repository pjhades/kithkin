#include <kernel/mm.h>

.code32

.globl _start
.section .text.entry
_start:
    /* fill the two page directory to map first 8MB memory */
    movl $pgdir0, %edi
    movl $(N_PTE_PER_PDE << 2), %ecx
    movl $(PTE_P | PTE_RW | PTE_US), %eax
1:
    test %ecx, %ecx
    jz 2f
    stosl
    addl $0x1000, %eax
    dec %ecx
    jmp 1b
2:
    movl $init_pagedir, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl  $CR0_PG, %eax
    movl %eax, %cr0
    jmp 3f
3:
    movl $kernel_main, %eax
    jmp *%eax

    movb $0x30, %al
    movb %al, %ds:(0x0b8000)
    movb $0x0c, %al
    movb %al, %ds:(0x0b8001)
    jmp .

/* initial page directory at physical address 0x101000 */
.org 0x1000
.globl init_pagedir
init_pagedir:
    .long 0x00102000 | PDE_P | PDE_RW | PDE_US
    .long 0x00103000 | PDE_P | PDE_RW | PDE_US
    .long 0x00104000 | PDE_P | PDE_RW | PDE_US
    .long 0x00105000 | PDE_P | PDE_RW | PDE_US
    .fill N_USER_PDE - 4, 4, 0
    .long 0x00102000 | PDE_P | PDE_RW | PDE_US
    .long 0x00103000 | PDE_P | PDE_RW | PDE_US
    .long 0x00104000 | PDE_P | PDE_RW | PDE_US
    .long 0x00105000 | PDE_P | PDE_RW | PDE_US
    .fill N_KERNEL_PDE - 4, 4, 0

/* 1st: physical [0, 4MB) */
.org 0x2000
.globl pgdir0
pgdir0:

/* 2nd: physical [4, 8MB)*/
.org 0x3000
.globl pgdir1
pgdir1:

/* 3rd: physical [8, 12MB)*/
.org 0x4000
.globl pgdir2
pgdir2:

/* 4th: physical [12, 16MB)*/
.org 0x5000
.globl pgdir3
pgdir3:

/* 4KB space holding data passed by loader */
.org 0x6000

.org 0x7000

/* page directory */
.section .bss
.align PAGE_SIZE
.globl pagedir
pagedir:

