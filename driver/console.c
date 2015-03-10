#include <asm/x86.h>
#include <kernel/types.h>
#include <kernel/console.h>

struct console vga;

static void cons_set_cursor(void)
{
    uint16_t pos = vga.c_row * VGA_COLS + vga.c_col;

    outb(VGA_CTRL, VGA_CURSOR_HI);
    outb(VGA_DATA, (pos >> 8) & 0xff);
    outb(VGA_CTRL, VGA_CURSOR_LO);
    outb(VGA_DATA, pos & 0xff);
}

static void cons_scroll(void)
{
    int i, j;
    vga_mem_ptr_t video = (vga_mem_ptr_t)VGA_MEM_DATA;

    for (i = 1; i < VGA_ROWS; i++)
        for (j = 0; j < VGA_COLS; j++)
            video[i - 1][j] = video[i][j];
    for (j = 0; j < VGA_COLS; j++)
        video[VGA_ROWS - 1][j] = 0;
    vga.c_row = VGA_ROWS - 1;
    vga.c_col = 0;
    cons_set_cursor();
}

static inline void cons_add_row(void)
{
    if (++vga.c_row == VGA_ROWS)
        cons_scroll();
    cons_set_cursor();
}

static inline void cons_add_col(void)
{
    if (++vga.c_col == VGA_COLS) {
        vga.c_col = 0;
        cons_add_row();
    }
    cons_set_cursor();
}

void cons_clear_screen(void)
{
    int i, j;
    vga_mem_ptr_t video = (vga_mem_ptr_t)VGA_MEM_DATA;

    vga.c_row = 0; 
    vga.c_col = 0;
    for (i = 0; i < VGA_ROWS; i++)
        for (j = 0; j < VGA_COLS; j++)
            video[i][j] = 0x0e<<8;
}

void cons_putchar(char ch)
{
    vga_mem_ptr_t video = (vga_mem_ptr_t)VGA_MEM_DATA;

    switch (ch) {
        case ' ':
            cons_add_col();
            break;
        case '\n':
            vga.c_col = 0;
            cons_add_row();
            break;
        case '\r':
            vga.c_col = 0;
            cons_set_cursor();
            break;
        default:
            video[vga.c_row][vga.c_col] = (0x0e << 8) | ch;
            cons_add_col();
            break;
    }
}

void cons_puts(const char *s)
{
    for (; *s; s++)
        cons_putchar(*s);
}

void cons_puthex(uint64_t hex)
{
    int i = 0, j = 2, count = 0;
    char *table = "0123456789abcdef", s[20], t[20] = "0x00";
    uint64_t v = hex;

    while (v) {
        s[i++] = table[(v & 0xf0) >> 4];
        s[i++] = table[v & 0x0f];
        v >>= 8;
    }
    --i;
    while (i > 0) {
        t[j] = s[i - 1];
        t[j + 1] = s[i];
        j += 2;
        i -= 2;
    }
    if (j == 2)
        j = 4;
    t[j] = '\0';
    cons_puts(t);
}
