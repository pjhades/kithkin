#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/console.h>

extern char _stextentry[], _etextentry[];

void kernel_main(void)
{
    console_clear_screen();

    printk("_stextentry=%p\n", _stextentry);
    printk("_etextentry=%p\n", _etextentry);

    while (1);
}
