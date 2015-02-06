asm (".code16gcc\n");

#include <arch/x86.h>
#include <include/common.h>

void print_string(char *s, int len);

static int check_a20(void)
{
    *((uint16_t *)0x07dfe) = 0xdead; /* write to 0000:7dfe */
    *((uint16_t *)((0x0ffff << 4) + 0x07e0e)) = 0xbeef; /* write to ffff:7e0e */
    if (*((uint16_t *)0x07dfe) == 0xbeef) /* check if overwritten */
        return 0;
    return 1;
}

static void wait_read_8042(void)
{
    uint8_t value;

    while (1) {
        value = inb(0x64);
        if (value & 0x01)
            return;
    }
}

static void wait_write_8042(void)
{
    uint8_t value;
    
    while (1) {
        value = inb(0x64);
        if (!(value & 0x02))
            return;
    }
}

static int enable_a20(void)
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
