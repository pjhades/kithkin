#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/mm.h>

struct console_device console;

void kernel_main(void)
{
    console_init(virt(CONSOLE_MEM_DATA));
    meminit();

    printk("i want to retire at 60\n");
    while (1);
}
