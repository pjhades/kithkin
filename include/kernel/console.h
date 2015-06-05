#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <kernel/types.h>

#define CONSOLE_CTRL      0x3d4
#define CONSOLE_DATA      0x3d5
#define CONSOLE_CURSOR_HI 0xe
#define CONSOLE_CURSOR_LO 0xf
#define CONSOLE_MEM_DATA  0x0b8000
#define CONSOLE_MEM_ATTR  0x0b8001
#define CONSOLE_ROWS      25
#define CONSOLE_COLS      80

typedef uint16_t (*console_mem_ptr_t)[CONSOLE_COLS];

struct console {
    uint8_t c_row;
    uint8_t c_col;
};

void cons_clear_screen(void);
void cons_putchar(char ch);
void cons_puts(const char *s);
void cons_puthex(uint64_t hex);

#endif
