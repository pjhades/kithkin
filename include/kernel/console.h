#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define VGA_CTRL      0x3d4
#define VGA_DATA      0x3d5
#define VGA_CURSOR_HI 0xe
#define VGA_CURSOR_LO 0xf
#define VGA_MEM_DATA  0x0b8000
#define VGA_MEM_ATTR  0x0b8001
#define VGA_ROWS      25
#define VGA_COLS      80

typedef uint16_t (*vga_mem_ptr_t)[VGA_COLS];

struct console {
    uint8_t c_row;
    uint8_t c_col;
};

void cons_clear_screen(void);
void cons_putchar(char ch);
void cons_puts(const char *s);
void cons_puthex(uint64_t hex);

#endif
