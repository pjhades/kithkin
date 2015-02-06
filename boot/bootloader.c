asm (".code16gcc\n");

#include <arch/x86.h>
#include <include/common.h>

void print_string(char *s, int len);

int check_a20(void)
{
    int ret;
    asm volatile (
            "pushl %%ds\n\t"
            "pushl %%es\n\t"
            "pushl %%edi\n\t"
            "pushl %%esi\n\t"
            "xorw %%ax, %%ax\n\t"
            "movw %%ax, %%es\n\t" /* base 0x0000*/
            "notw %%ax\n\t"
            "movw %%ax, %%ds\n\t" /* base 0xffff */
            "movw $0x7dfe, %%di\n\t" /* bootsector magic */
            "movw $0x7e0e, %%si\n\t" /* 1M higher */
            "movw $0xdead, %%es:(%%di)\n\t" /* write to 0000:7dfe */
            "movw $0xbeef, %%ds:(%%si)\n\t" /* write to ffff:7e0e */
            "cmpw $0xbeef, %%es:(%%di)\n\t" /* test if overwritten */
            "movb $0x0, %%al\n\t"
            "jz .check_a20_done\n\t"
            "movb $0x01, %%al\n\t"
        ".check_a20_done:\n\t"
            "popl %%esi\n\t"
            "popl %%edi\n\t"
            "popl %%es\n\t"
            "popl %%ds\n\t"
            :"=a"(ret)
            ::
            );
    return ret;
}

void wait_read_8042(void)
{
    uint8_t value;

    while (1) {
        value = inb(0x64);
        if (value & 0x01)
            return;
    }
}

void wait_write_8042(void)
{
    uint8_t value;
    
    while (1) {
        value = inb(0x64);
        if (!(value & 0x02))
            return;
    }
}

int enable_a20(void)
{
    uint8_t value;

    if (check_a20())
        return 1;

    /* try BIOS service */
    asm volatile (
            "movw $0x2401, %ax\n\t"
            "int $0x15\n\t"
            );

    if (check_a20())
        return 1;

    /* try 8042 keyboard controller */
    wait_write_8042();
    outb(0x64, 0xad); /* disable keyboard */
    wait_write_8042();
    outb(0x64, 0xd0);
    wait_read_8042();
    value = inb(0x60);
    wait_write_8042();
    outb(0x64, 0xd1);
    wait_write_8042();
    outb(0x60, value | 0x02); /* enable A20 */
    wait_write_8042();
    outb(0x64, 0xae); /* enable keyboard */
    wait_write_8042();

    if (check_a20())
        return 1;

    /* try fast A20 gate */
    value = inb(0x92);
    outb(0x92, value | 0x02);
            
    if (check_a20())
        return 1;
    return 0;
}

void main(void)
{
    print_string("Enabling A20 line ...", 21);

    if (enable_a20())
        print_string("done\r\n", 6);
    else
        print_string("failed\r\n", 8);
    while (1);
}
