.code16
.globl _start

.text
_start:
    pushw $21
    pushw $str_a20
    call print_string
    popw %ax
    popw %ax

    /* open A20, see http://wiki.osdev.org/A20 */
    call enable_a20
    call check_a20
    testb %al, %al
    jnz .a20_enabled
    pushw $6
    pushw $str_failed
    call print_string
    popw %ax
    popw %ax
    jmp .a20_done
.a20_enabled:
    pushw $4
    pushw $str_done
    call print_string
    popw %ax
    popw %ax
.a20_done:
    call newline
    
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
    addb $0x01, %dh /* move to next line first column */
    xorb %dl, %dl
    movb $0x02, %ah /* set new cursor position*/
    xorb %bh, %bh
    int $0x10
    ret


/* check if A20 line is enabled, 
 * al == 1 if enabled, al == 0 otherwise
 */
check_a20:
    pushw %ds
    pushw %es
    pushw %di
    pushw %si
    xorw %ax, %ax
    movw %ax, %es /* base 0x0000*/
    notw %ax
    movw %ax, %ds /* base 0xffff */
    movw $0x7dfe, %di /* bootsector magic */
    movw $0x7e0e, %si /* 1M higher */
    movw $0xdead, %es:(%di) /* write to 0000:7dfe */
    movw $0xbeef, %ds:(%si) /* write to ffff:7e0e */
    cmpw $0xbeef, %es:(%di) /* test if overwritten */
    movb $0x0, %al
    jz .check_a20_done
    movb $0x01, %al
.check_a20_done:
    popw %si
    popw %di
    popw %es
    popw %ds
    ret


/* wait until keyboard controller is readable */
wait_read_8042:
    inb $0x64, %al
    testb $0x01, %al
    jz wait_read_8042
    ret


/* wait until keyboard controller is writable */
wait_write_8042:
    inb $0x64, %al
    testb $0x02, %al
    jnz wait_write_8042
    ret


/* enable A20 line,
 * al == 1 if enabled, al == 0 otherwise
 */
enable_a20:
    call check_a20
    testb %al, %al
    jnz .enable_a20_done

    /* try BIOS service */
    movw $0x2401, %ax
    int $0x15

    call check_a20
    testb %al, %al
    jnz .enable_a20_done

    /* try keyboard controller */
    call wait_write_8042
    movb $0xad, %al
    outb %al, $0x64 /* disable keyboard */
    call wait_write_8042
    movb $0xd0, %al
    outb %al, $0x64 /* read buffer */
    call wait_read_8042
    inb $0x60, %al
    pushw %ax
    call wait_write_8042
    movb $0xd1, %al
    outb %al, $0x64 /* write buffer */
    call wait_write_8042
    popw %ax
    orb $0x02, %al
    outb %al, $0x60 /* enable A20 */
    call wait_write_8042
    movb $0xae, %al
    outb %al, $0x64 /* enable keyboard */
    call wait_write_8042

    call check_a20
    testb %al, %al
    jnz .enable_a20_done

    /* try fast A20 gate */
    inb $0x92, %al
    orb $0x02, %al
    outb %al, $0x92

    call check_a20
    testb %al, %al
    jnz .enable_a20_done

    movb $0x0, %al
    ret
.enable_a20_done:
    movb $0x01, %al
    ret



str_a20:            .ascii "Enabling A20 line ..."
str_run_bootloader: .ascii "Run bootloader ..."
str_failed:         .ascii "failed"
str_done:           .ascii "done"
