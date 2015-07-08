#include <kernel/buddy.h>

struct buddy_data buddy;

void init_buddy(void)
{
    int order;

    for (order = 0; order <= MAX_ORDER; order++) {
        list_init(&buddy.free_areas[order].pages);
        buddy.free_areas[order].size = 0;
    }
}

void free_pages(struct page *ptr, int order)
{
}
