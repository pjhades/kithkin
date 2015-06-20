#include <kernel/mm.h>

.code32

.macro set_pde addr, ptaddr
    movl pa($\ptaddr), %eax
    orl $(PDE_P | PDE_RW | PDE_US), %eax
    movl pa($\addr), %edi
    movl %eax, (%edi)
.endm

.macro set_empty_pde addr, count
    movl $\count, %ecx
    movl pa($\addr), %edi
    xorl %eax, %eax
1:
    stosl
    decl %ecx
    test %ecx, %ecx
    jnz 1b
.endm

.globl _start
.section .text.entry
_start:
    /* page tables mapping the lower part */
    set_pde addr=init_pagedir,    ptaddr=pagetable0
    set_pde addr=init_pagedir+4,  ptaddr=pagetable1
    set_pde addr=init_pagedir+8,  ptaddr=pagetable2
    set_pde addr=init_pagedir+12, ptaddr=pagetable3
    /* empty */
    set_empty_pde addr=init_pagedir+16, count=N_USER_PDE-4
    /* page tables mapping the higher part */
    set_pde addr=init_pagedir+N_USER_PDE*4,    ptaddr=pagetable0
    set_pde addr=init_pagedir+N_USER_PDE*4+4,  ptaddr=pagetable1
    set_pde addr=init_pagedir+N_USER_PDE*4+8,  ptaddr=pagetable2
    set_pde addr=init_pagedir+N_USER_PDE*4+12, ptaddr=pagetable3
    /* empty */
    set_empty_pde addr=init_pagedir+N_USER_PDE*4+16, count=N_KERNEL_PDE-4

    /* fill the page tables */
    movl pa($pagetable0), %edi
    movl $(N_PTE_PER_PDE << 2), %ecx
    movl $(PTE_P | PTE_RW | PTE_US), %eax
2:
    test %ecx, %ecx
    jz 3f
    stosl
    addl $0x1000, %eax
    decl %ecx
    jmp 2b
3:
    movl pa($init_pagedir), %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl  $CR0_PG, %eax
    movl %eax, %cr0
    jmp 4f
4:
    movl $kernel_main, %eax
    jmp *%eax

    movb $0x30, %al
    movb %al, %ds:(0x0b8000)
    movb $0x0c, %al
    movb %al, %ds:(0x0b8001)
    jmp .


.section .bss
.align PAGE_SIZE
.globl init_pagedir
init_pagedir:
    .fill 1024, 4, 0

.globl pagetable0
pagetable0:
    .fill 1024, 4, 0

.globl pagetable1
pagetable1:
    .fill 1024, 4, 0

.globl pagetable2
pagetable2:
    .fill 1024, 4, 0

.globl pagetable3
pagetable3:
    .fill 1024, 4, 0

.globl pagedir
pagedir:

