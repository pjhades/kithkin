#include <asm/e820.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/console.h>

int test;

void kernel_main(void)
{
    console_clear_screen();

    cputs("address of test: ");
    cputhex((uint32_t)&test);
    cputs("\n");
    cputs("hello world!\n");

    int a = printk("%d + %d = %d\n", 199, 145, 199 + 145);
    cputhex(a);
    printk("\n");
    unsigned int b = 0x80000000;
    printk("i: %d\nu: %u\n", b, b);
    uint64_t c = 0x8000000000000000LL;
    printk("q: %q\nU: %U\n", c, c);

    while (1);
}
