#include <string.h>
#include <kernel/kernel.h>
#include <kernel/bootmem.h>
#include <kernel/mm.h>
#include <kernel/buddy.h>

struct bootmem_data bdata;

static void bootmem_mark_usable(struct bootmem_data *bd, u32 pfn)
{
    bd->bitmap[pfn >> 3] &= ~(1 << (pfn & 7));
}

static void bootmem_mark_reserved(struct bootmem_data *bd, u32 pfn)
{
    bd->bitmap[pfn >> 3] |= (1 << (pfn & 7));
}

static int bootmem_is_reserved(struct bootmem_data *bd, u32 pfn)
{
    return bd->bitmap[pfn >> 3] & (1 << (pfn & 7));
}

void init_bootmem(void)
{
    u32 pfn;
    int nr_bitmap_page;

    /* bitmap lives in the next page after _end */
    bdata.bitmap = (unsigned char *)virt(pfn_up(phys(_end)) << PAGE_SHIFT);
    bdata.size = (maxpfn + 7) >> 3;
    nr_bitmap_page = (bdata.size + PAGE_SIZE - 1) >> PAGE_SHIFT;

    /* start allocation from the page after the bitmap */
    bdata.lastpfn = pfn_up((pfn_up(phys(_end)) << PAGE_SHIFT) + bdata.size);
    bdata.last = (void *)virt(bdata.lastpfn << PAGE_SHIFT);

    printk("_end=%p\n"
           "bootmem: bitmap=%p (%d bytes, %d pages)\n"
           "bootmem: lastpfn=%p, last=%p\n",
           _end, bdata.bitmap, bdata.size, nr_bitmap_page, bdata.lastpfn,
           bdata.last);

    /* mark all pages as reserved */
    memset(bdata.bitmap, 0xff, bdata.size);

    /* mark usable pages */
    for (pfn = pfn_up(phys(_end)); pfn <= maxpfn; pfn++)
        bootmem_mark_usable(&bdata, pfn);

    /* mark pages used by bootmem as reserved */
    pfn = phys_to_pfn(phys(bdata.bitmap));
    while (nr_bitmap_page--) {
        bootmem_mark_reserved(&bdata, pfn);
        pfn++;
    }
}

void *bootmem_alloc(u32 size)
{
    void *ret = NULL;
    int nr_page, i;
    u32 pfn, bytes, n;

    if (size >= PAGE_SIZE) {
        nr_page = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
        if (bdata.lastpfn + nr_page - 1 > maxpfn)
            die("cannot afford %d pages, "
                "last=%p, lastpfn=%x\n", nr_page, bdata.last, bdata.lastpfn);

        /* bytes used in last page */
        bytes = size & PAGE_MASK;

        /* not start from a new page */
        if (phys(bdata.last) != (bdata.lastpfn << PAGE_SHIFT))
            bootmem_mark_reserved(&bdata, bdata.lastpfn);

        ret = (void *)virt(pfn_up(phys(bdata.last)) << PAGE_SHIFT);
        pfn = pfn_down(phys(ret));

        for (i = 0; i < nr_page - 1; i++, pfn++)
            bootmem_mark_reserved(&bdata, pfn);

        /* mark last page if required size is a multiple of pages */
        if (bytes == 0) {
            bootmem_mark_reserved(&bdata, pfn);
            pfn++;
        }

        bdata.lastpfn = pfn;
        bdata.last = (void *)virt((pfn << PAGE_SHIFT) + bytes);

        if (!ret)
            die("failed allocating %d pages\n", nr_page);
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

void free_all_bootmem(void)
{
    int pfn, nr_bitmap_page;

    /* reserve the last page we haven't used up */
    if ((u32)bdata.last > virt(pfn_to_phys(bdata.lastpfn)))
        bootmem_mark_reserved(&bdata, bdata.lastpfn);

    for (pfn = minpfn; pfn <= maxpfn; pfn++) {
        if (!bootmem_is_reserved(&bdata, pfn)) {
            free_pages(pfn_to_page(pfn), 0);
        }
    }

    /* free the pages used by bootmem */
    nr_bitmap_page = (bdata.size + PAGE_SIZE - 1) >> PAGE_SHIFT;
    pfn = phys_to_pfn(phys(bdata.bitmap));
    while (nr_bitmap_page--) {
        free_pages(pfn_to_page(pfn), 0);
        pfn++;
    }
}
