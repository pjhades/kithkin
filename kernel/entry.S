#include <kernel/mm.h>

.code32

.globl _start
.text
_start:
    /* fill the two page directory to map first 8MB memory */
    movl $pgdir0, %edi
    movl $(N_PTE_PER_PDE << 1), %ecx
    movl $(PTE_P | PTE_RW | PTE_US), %eax
1:
    test %ecx, %ecx
    jz 2f
    stosl
    addl $0x1000, %eax
    dec %ecx
    jmp 1b
2:
    movb $0x30, %al
    movb %al, %ds:(0x0b8000)
    movb $0x0c, %al
    movb %al, %ds:(0x0b8001)
    jmp .

/* initial page directory at physical address 0x101000 */
.org 0x1000
.globl page_directory
page_directory:
    .long 0x00102000 | PDE_P | PDE_RW | PDE_US
    .long 0x00103000 | PDE_P | PDE_RW | PDE_US
    .fill N_USER_PDE - 2, 4, 0
    .long 0x00102000 | PDE_P | PDE_RW | PDE_US
    .long 0x00103000 | PDE_P | PDE_RW | PDE_US
    .fill N_KERNEL_PDE - 2, 4, 0

/* 1st page directory: physical [0, 4MB) */
.org 0x2000
.globl pgdir0
pgdir0:

/* 2nd page directory: physical [4, 8MB)*/
.org 0x3000
.globl pgdir1
pgdir1:
