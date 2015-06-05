#include <asm/x86.h>
#include <kernel/types.h>
#include <kernel/console.h>

static struct console cons;

static void cons_set_cursor(void)
{
    uint16_t pos = cons.c_row * CONSOLE_COLS + cons.c_col;

    outb(CONSOLE_CTRL, CONSOLE_CURSOR_HI);
    outb(CONSOLE_DATA, (pos >> 8) & 0xff);
    outb(CONSOLE_CTRL, CONSOLE_CURSOR_LO);
    outb(CONSOLE_DATA, pos & 0xff);
}

static void cons_scroll(void)
{
    int i, j;
    console_mem_ptr_t video = (console_mem_ptr_t)CONSOLE_MEM_DATA;

    for (i = 1; i < CONSOLE_ROWS; i++)
        for (j = 0; j < CONSOLE_COLS; j++)
            video[i - 1][j] = video[i][j];
    for (j = 0; j < CONSOLE_COLS; j++)
        video[CONSOLE_ROWS - 1][j] = 0;
    cons.c_row = CONSOLE_ROWS - 1;
    cons.c_col = 0;
    cons_set_cursor();
}

static inline void cons_add_row(void)
{
    if (++cons.c_row == CONSOLE_ROWS)
        cons_scroll();
    cons_set_cursor();
}

static inline void cons_add_col(void)
{
    if (++cons.c_col == CONSOLE_COLS) {
        cons.c_col = 0;
        cons_add_row();
    }
    cons_set_cursor();
}

void cons_clear_screen(void)
{
    int i, j;

    console_mem_ptr_t video = (console_mem_ptr_t)CONSOLE_MEM_DATA;

    cons.c_row = 0; 
    cons.c_col = 0;
    for (i = 0; i < CONSOLE_ROWS; i++)
        for (j = 0; j < CONSOLE_COLS; j++)
            video[i][j] = 0x07<<8;
}

void cons_putchar(char ch)
{
    console_mem_ptr_t video = (console_mem_ptr_t)CONSOLE_MEM_DATA;

    switch (ch) {
        case ' ':
            cons_add_col();
            break;
        case '\n':
            cons.c_col = 0;
            cons_add_row();
            break;
        case '\r':
            cons.c_col = 0;
            cons_set_cursor();
            break;
        default:
            video[cons.c_row][cons.c_col] = (0x07 << 8) | ch;
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
    int i = 0, j = 2;
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
