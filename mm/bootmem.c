#include <string.h>
#include <kernel/alloc.h>
#include <kernel/mm.h>
#include <kernel/kernel.h>

struct bootmem_data bdata;
extern char _end[];
extern uint32_t minpfn, maxpfn;

void init_bootmem(void)
{
    uint32_t pfn;

    /* bitmap lives in the next page after _end */
    bdata.bitmap = (char *)(pfn_up(_end) << PAGE_SHIFT);
    bdata.size = ((maxpfn + 7) >> 3) + 10000;
    /* start allocation from the page after the bitmap */
    bdata.lastpfn = pfn_up((pfn_up(_end) << PAGE_SHIFT) + bdata.size);
    bdata.last = (char *)(bdata.lastpfn << PAGE_SHIFT);
    printk("bootmem: _end=%p\n"
           "bootmem: bitmap=%p, size=%x, lastpfn=%p, last=%p\n",
           _end, bdata.bitmap, bdata.size, bdata.lastpfn, bdata.last);

    /* mark all pages reserved */
    memset(bdata.bitmap, 0xff, bdata.size);

    /* mark usable pages */
    for (pfn = minpfn; pfn <= maxpfn; pfn++)
        bootmem_mark_usable(bdata, pfn);

    /* mark pages used by bootmem reserved */
    bootmem_mark_reserved(bdata, pfn_up(_end));
}

