.code16
.globl _start

.text
_start:
    pushw $18
    pushw $str_run_bootloader
    call print_string
    popw %ax
    popw %ax

    jmp .

/* print_string(string, length) */
print_string:
    pushw %cx
    pushw %bp 

    movb $0x03, %ah /* get cursor position */
    xorb %bh, %bh /* page */
    int $0x10

    movw 0x8(%esp), %cx /* arg: length */
    movw 0x6(%esp), %bp /* arg: string */

    movb $0x13, %ah
    movb $0x01, %al /* string contains no attributes, update cursor */
    xorb %bh, %bh /* page */
    movb $0x07, %bl /* attribute */
    int $0x10

    popw %bp
    popw %cx
    ret

/* newline() */
newline:
    movb $0x03, %ah /* get cursor position */
    xorb %bh, %bh /* page */
    int $0x10
    addb $0x01, %dl /* move to next line first column */
    xorb %dl, %dl
    movb $0x02, %ah /* set new cursor position*/
    xorb %bh, %bh
    int $0x10
    ret

str_run_bootloader:
.ascii "Run bootloader ..."
str_failed:
.ascii "failed"
str_done:
.ascii "done"
