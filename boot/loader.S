#include <kernel/mmu.h>

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


.globl jump_to_protected_mode
jump_to_protected_mode:
    ljmp    $BOOT_CS, $pm_entry

.equ stack, 0x07ffff

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

