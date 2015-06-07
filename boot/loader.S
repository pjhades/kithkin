#include <asm/mmu.h>

#define BOOT_STACK 0x07ffff

.globl _start

.text
.code16
_start:
    jmp main


.globl print_string
print_string:
    pushl %ecx
    pushl %ebp 

    movb $0x03, %ah /* get cursor position */
    xorb %bh, %bh /* page */
    int $0x10

    movw 0x10(%esp), %cx /* arg: length */
    movw 0x0c(%esp), %bp /* arg: string */

    movb $0x13, %ah
    movb $0x01, %al /* string contains no attributes, update cursor */
    xorb %bh, %bh /* page */
    movb $0x07, %bl /* attribute */
    int $0x10

    popl %ebp
    popl %ecx
    ret


.global enter_protected_mode
enter_protected_mode:
    lgdt boot_gdtptr
    movl %cr0, %eax
    orl $CR0_PM, %eax
    movl %eax, %cr0
    ljmp $BOOT_CS, $pm_entry


.equ stack, BOOT_STACK

.code32
pm_entry:
    movw $BOOT_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $stack, %esp
    call pm_main
    jmp .

