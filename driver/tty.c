#include <common.h>
#include <driver/tty.h>

extern struct console vga;

void pm_printc(char ch)
{
    if (ch == ' ') {
        if (++vga.c_col == 80)
            goto newline;
        return;
    }
    if (ch == '\n') {
newline:
        ++vga.c_row;
        vga.c_col = 0;
        return;
    }
    if (ch == '\r') {
        vga.c_col = 0;
        return;
    }
    *((uint8_t *)VGA_MEM_DATA + ((vga.c_row*80+vga.c_col)<<1)) = ch;
    *((uint8_t *)VGA_MEM_ATTR + ((vga.c_row*80+vga.c_col)<<1)) = 0x0e;
    if (++vga.c_col == 80) {
        vga.c_col = 0;
        ++vga.c_row;
    }
}

// TODO replace this with console functions
void pm_print(char *s, int len)
{
    for (; len; s++, len--)
        pm_printc(*s);
}

// TODO replace this with console functions
void pm_printhex(uint32_t hex, int unit)
{
    /* unit = 1, 2, 4 */
    char *table = "0123456789abcdef";
    uint32_t v32;
    uint16_t v16;
    uint8_t v8;

    if (unit == 1) {
        if (hex == 0)
            pm_print("00 ", 3);
        else {
            v32 = hex;
            while (v32) {
                v8 = v32 % 256;
                pm_printc(table[(v8 & 0xf0) >> 4]);
                pm_printc(table[v8 & 0x0f]);
                pm_printc(' ');
                v32 >>= 8;
            }
        }
    }
    else if (unit == 2) {
        if (hex == 0)
            pm_print("0000 ", 5);
        else {
            v32 = hex;
            while (v32) {
                v16 = v32 % 65536;
                pm_printc(table[(v16 & 0xf000) >> 12]);
                pm_printc(table[(v16 & 0x0f00) >> 8]);
                pm_printc(table[(v16 & 0x00f0) >> 4]);
                pm_printc(table[v16 & 0x0f]);
                pm_printc(' ');
                v32 >>= 16;
            }
        }
    }
    else {
        if (hex == 0)
            pm_print("00000000 ", 9);
        else {
            v32 = hex;
            pm_printc(table[(v32 & 0xf0000000) >> 28]);
            pm_printc(table[(v32 & 0x0f000000) >> 24]);
            pm_printc(table[(v32 & 0x00f00000) >> 20]);
            pm_printc(table[(v32 & 0x000f0000) >> 16]);
            pm_printc(table[(v32 & 0x0000f000) >> 12]);
            pm_printc(table[(v32 & 0x00000f00) >> 8]);
            pm_printc(table[(v32 & 0x000000f0) >> 4]);
            pm_printc(table[v32 & 0x0f]);
            pm_printc(' ');
        }
    }
}
