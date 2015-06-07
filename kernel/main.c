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

    while (1);
}
