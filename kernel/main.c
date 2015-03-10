#include <kernel/types.h>
#include <kernel/console.h>

int test;
// FIXME
// vga is loaded at a another address with the file console.o,
// so the last print position cannot be preserved.
// fix this

void kernel_main(void)
{
    cons_clear_screen();

    cons_puts("address of test: ");
    cons_puthex((uint32_t)&test);
    cons_puts("\n");
    cons_puts("hello world!\n");
    while (1);
}
