#ifndef __TTY_H__
#define __TTY_H__

#define VGA_CTRL 0x3d4
#define VGA_DATA 0x3d5
#define VGA_CURSOR_HI 0xe
#define VGA_CURSOR_LO 0xf
#define VGA_MEM_DATA 0x0b8000
#define VGA_MEM_ATTR 0x0b8001

struct console {
    uint8_t c_row;
    uint8_t c_col;
};

#endif
