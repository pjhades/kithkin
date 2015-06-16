#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/mm.h>

extern char _stextentry[], _etextentry[];

void kernel_main(void)
{
    console_clear_screen();

    meminit();

    while (1);
}
