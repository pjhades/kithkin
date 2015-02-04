.code16

.text
.globl _start
_start:
    cli

    movw %cs, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw $stack, %sp

    /* clear screen */
    xorb %ah, %ah
    movb $0x03, %al
    int $0x10

    /* print status */
    pushw $22
    pushw $str_loading_bootloader
    call print_string
    popw %ax
    popw %ax

    /* load bootloader on C:H:S=0:0:2 */
    movw $0x5, %si /* try 5 times */
int0x13_retry:
    test %si, %si
    jz int0x13_failed

    decw %si

    xorb %ah, %ah
    movb $0x80, %dl
    int $0x13

    movb $0x02, %ah
    movb $0x01, %al /* number of sectors to read */
    xorb %ch, %ch /* track low 8 bits */
    movb $0x02, %cl /* sector, track high 2 bits */
    xorb %dh, %dh /* head */
    movb $0x80, %dl /* drive */
    movw $bootloader, %bx /* buffer */
    int $0x13

    jnc int0x13_done
    jmp int0x13_retry

int0x13_failed:
    pushw $6
    pushw $str_failed
    call print_string
    popw %ax
    popw %ax
    jmp .

int0x13_done:
    pushw $4
    pushw $str_done
    call print_string
    popw %ax
    popw %ax

    call newline

    movw $bootloader, %bx
    jmp *%bx

.equ stack, 0x7c00
.equ bootloader, 0x500

/* print_string(string, length) */
print_string:
    pushw %cx
    pushw %bp 

    movb $0x03, %ah /* get cursor position */
    xorb %bh, %bh /* page */
    int $0x10

    movw 0x08(%esp), %cx /* arg: length */
    movw 0x06(%esp), %bp /* arg: string */

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
    addb $0x01, %dh /* move to next line first column */
    xorb %dl, %dl
    movb $0x02, %ah /* set new cursor position*/
    xorb %bh, %bh
    int $0x10
    ret


str_loading_bootloader:
.ascii "Loading bootloader ..."
str_failed:
.ascii "failed"
str_done:
.ascii "done"

.org _start + 510
.word 0xaa55
