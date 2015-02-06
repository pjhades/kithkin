.code16
.globl _start

.text
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

