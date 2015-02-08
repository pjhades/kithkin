#include <arch/mmu.h>

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
    ljmp    $BOOT_GDT_CODE<<3, $protected_mode_entry

/* leave this here for debugging */
/*
gdt:
  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00 
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00 

gdtdesc:
  .word   0x17
  .long   gdt
*/
    

.code32
protected_mode_entry:
    movw $BOOT_GDT_DATA<<3, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movb $0x30, %al
    movb %al, %ds:(0x0b8000)
    movb $0x0e, %al
    movb %al, %ds:(0x0b8001)

    jmp .
