.code32

.globl _start
.text
_start:
    movb $0x30, %al
    movb %al, %ds:(0x0b8000)
    movb $0x0e, %al
    movb %al, %ds:(0x0b8001)
    jmp .
