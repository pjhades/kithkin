#ifndef __LOADER_H__
#define __LOADER_H__

#define BOOT_GDT_ENTRY_CODE 1
#define BOOT_GDT_ENTRY_DATA 2

#define BOOT_CS (BOOT_GDT_ENTRY_CODE << 3)
#define BOOT_DS (BOOT_GDT_ENTRY_DATA << 3)

#define LOADER_STACK 0x07ffff

#endif
