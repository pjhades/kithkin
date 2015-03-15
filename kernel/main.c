#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/console.h>

int test;

void kernel_main(void)
{
    cons_clear_screen();

    cons_puts("address of test: ");
    cons_puthex((uint32_t)&test);
    cons_puts("\n");
    cons_puts("hello world!\n");

    int a = printk("%d + %d = %d\n", 199, 145, 199 + 145);
    cons_puthex(a);

    while (1);
}
