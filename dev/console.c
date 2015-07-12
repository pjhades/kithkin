#include <console.h>
#include <asm.h>
#include <kernel/types.h>

extern struct console_device console;

static void console_set_cursor(void)
{
    u16 pos = console.row * CONSOLE_COLS + console.col;

    outb(CONSOLE_CTRL, CONSOLE_CURSOR_HI);
    outb(CONSOLE_DATA, (pos >> 8) & 0xff);
    outb(CONSOLE_CTRL, CONSOLE_CURSOR_LO);
    outb(CONSOLE_DATA, pos & 0xff);
}

static void console_scroll(void)
{
    int i, j;

    for (i = 1; i < CONSOLE_ROWS; i++)
        for (j = 0; j < CONSOLE_COLS; j++)
            console.mem[i - 1][j] = console.mem[i][j];
    for (j = 0; j < CONSOLE_COLS; j++)
        console.mem[CONSOLE_ROWS - 1][j] = (0x07 << 8) | ' ';
    console.row = CONSOLE_ROWS - 1;
    console.col = 0;
    console_set_cursor();
}

static inline void console_add_row(void)
{
    if (++console.row == CONSOLE_ROWS)
        console_scroll();
    console_set_cursor();
}

static inline void console_add_col(void)
{
    if (++console.col == CONSOLE_COLS) {
        console.col = 0;
        console_add_row();
    }
    console_set_cursor();
}

void console_clear_screen(void)
{
    int i, j;

    for (i = 0; i < CONSOLE_ROWS; i++)
        for (j = 0; j < CONSOLE_COLS; j++)
            console.mem[i][j] = 0x07<<8;
    console.row = 0;
    console.col = 0;
    console_set_cursor();
}

void console_init(u32 mem)
{
    console.mem = (console_mem_ptr_t)mem;
    console_clear_screen();
}

void cputchar(char ch)
{
    switch (ch) {
        case ' ':
            console_add_col();
            break;
        case '\n':
            console.col = 0;
            console_add_row();
            break;
        case '\r':
            console.col = 0;
            console_set_cursor();
            break;
        default:
            console.mem[console.row][console.col] = (0x07 << 8) | ch;
            console_add_col();
            break;
    }
}

void cputs(const char *s)
{
    for (; *s; s++)
        cputchar(*s);
}

void cputhex(u64 hex)
{
    int i = 0, j = 2;
    char *table = "0123456789abcdef", s[20], t[20] = "0x00";
    u64 v = hex;

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
    cputs(t);
}
