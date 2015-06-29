#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/mm.h>

void kernel_main(void)
{
    console_clear_screen();
    meminit();
    while (1);
}
