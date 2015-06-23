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
    bdata.last = (void *)(bdata.lastpfn << PAGE_SHIFT);
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

void *bootmem_alloc(uint32_t size)
{
    void *ret;
    int n_page, i;
    uint32_t pfn, bytes;

    if (size >= PAGE_SIZE) {
        n_page = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
        if (bdata.lastpfn + n_page - 1 > maxpfn)
            die("bootmem: cannot allocate %d pages. last=%p, lastpfn=%x\n",
                    n_page, bdata.last, bdata.lastpfn);

        /* bytes used in last page */
        bytes = size & PAGE_MASK;

        /* not start from a new page */
        if (bdata.last != (void *)(bdata.lastpfn << PAGE_SHIFT))
            bootmem_mark_reserved(bdata, bdata.lastpfn);

        ret = (void *)(pfn_up(bdata.last) << PAGE_SHIFT);
        pfn = pfn_down(ret);

        for (i = 0; i < n_page - 1; i++, pfn++)
            bootmem_mark_reserved(bdata, pfn);

        bdata.lastpfn = pfn;
        bdata.last = (void *)((pfn << PAGE_SHIFT) + bytes);

        return ret;
    }

    /* bytes left in current page */
    ret = bdata.last;
    bytes = PAGE_SIZE - ((uint32_t)bdata.last - (bdata.lastpfn << PAGE_SHIFT));
    if (size >= bytes) {
        size -= bytes;
        bootmem_mark_reserved(bdata, bdata.lastpfn);
        bdata.lastpfn++;
        bdata.last = (void *)pfn_down(bdata.lastpfn);
    }
    bdata.last += size;

    return ret;
}

