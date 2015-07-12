#ifndef __PM_H__
#define __PM_H__

#define BOOT_GDT_ENTRY_CODE 1
#define BOOT_GDT_ENTRY_DATA 2
#define NR_BOOT_GDT_ENTRY    3

#define BOOT_CS (BOOT_GDT_ENTRY_CODE << 3)
#define BOOT_DS (BOOT_GDT_ENTRY_DATA << 3)
#define BOOT_STACK 0x7ffff

#endif
