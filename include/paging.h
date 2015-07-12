#ifndef __PAGING_H__
#define __PAGING_H__

#define PDE_P   0x1
#define PDE_RW  0x2
#define PDE_US  0x4
#define PDE_PWT 0x8
#define PDE_PCD 0x10
#define PDE_A   0x20

#define PTE_P   0x1
#define PTE_RW  0x2
#define PTE_US  0x4
#define PTE_PWT 0x8
#define PTE_PCD 0x10
#define PTE_A   0x20
#define PTE_D   0x40
#define PTE_PAT 0x80
#define PTE_G   0x100

#define PAGEDIR_SHIFT 22
#define PAGE_SHIFT    12
#define PAGE_SIZE     (1 << PAGE_SHIFT)
#define PAGE_MASK     (PAGE_SIZE - 1)
#define NR_PDE         1024
#define NR_PTE_PER_PDE 1024

#endif
