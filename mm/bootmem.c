#include <string.h>
#include <kernel/bootmem.h>
#include <kernel/mm.h>
#include <kernel/kernel.h>

struct bootmem_data bdata;
extern char _end[];
extern u32 minpfn, maxpfn;

static void bootmem_mark_usable(struct bootmem_data *bdata, u32 pfn)
{
    bdata->bitmap[pfn >> 3] &= ~(1 << (pfn & 7));
}

static void bootmem_mark_reserved(struct bootmem_data *bdata, u32 pfn)
{
    bdata->bitmap[pfn >> 3] |= (1 << (pfn & 7));
}

void init_bootmem(void)
{
    u32 pfn;

    /* bitmap lives in the next page after _end */
    bdata.bitmap = (unsigned char *)(pfn_up(_end) << PAGE_SHIFT);
    bdata.size = (maxpfn + 7) >> 3;
    /* start allocation from the page after the bitmap */
    bdata.lastpfn = pfn_up((pfn_up(phys(_end)) << PAGE_SHIFT) + bdata.size);
    bdata.last = (void *)virt(bdata.lastpfn << PAGE_SHIFT);

    printk("bootmem: _end=%p\n"
           "bootmem: bitmap=%p, size=%x, lastpfn=%x, last=%p\n",
           _end, bdata.bitmap, bdata.size, bdata.lastpfn, bdata.last);

    /* mark all pages reserved */
    memset(bdata.bitmap, 0xff, bdata.size);

    /* mark usable pages */
    for (pfn = minpfn; pfn <= maxpfn; pfn++)
        bootmem_mark_usable(&bdata, pfn);

    /* mark pages used by bootmem reserved */
    bootmem_mark_reserved(&bdata, pfn_up(phys(_end)));
}

void *bootmem_alloc(u32 size)
{
    void *ret = NULL;
    int n_page, i;
    u32 pfn, bytes, n;

    if (size >= PAGE_SIZE) {
        n_page = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
        if (bdata.lastpfn + n_page - 1 > maxpfn)
            die("cannot afford %d pages, "
                "last=%p, lastpfn=%x\n", n_page, bdata.last, bdata.lastpfn);

        /* bytes used in last page */
        bytes = size & PAGE_MASK;

        /* not start from a new page */
        if (phys(bdata.last) != (bdata.lastpfn << PAGE_SHIFT))
            bootmem_mark_reserved(&bdata, bdata.lastpfn);

        ret = (void *)(pfn_up(bdata.last) << PAGE_SHIFT);
        pfn = pfn_down(phys(ret));

        for (i = 0; i < n_page - 1; i++, pfn++)
            bootmem_mark_reserved(&bdata, pfn);

        /* mark last page if required size is a multiple of pages */
        if (bytes == 0) {
            bootmem_mark_reserved(&bdata, pfn);
            pfn++;
        }

        bdata.lastpfn = pfn;
        bdata.last = (void *)virt((pfn << PAGE_SHIFT) + bytes);

        if (!ret)
            die("failed allocating %d pages\n", n_page);
        return ret;
    }

    /* bytes left in current page */
    n = size;
    ret = bdata.last;
    bytes = PAGE_SIZE - (phys(bdata.last) - (bdata.lastpfn << PAGE_SHIFT));

    if (size >= bytes) {
        size -= bytes;
        bootmem_mark_reserved(&bdata, bdata.lastpfn);
        bdata.lastpfn++;
        bdata.last = (void *)virt(bdata.lastpfn << PAGE_SHIFT);
    }
    bdata.last += size;

    if (!ret)
        die("failed allocating %d bytes\n", n);
    return ret;
}

