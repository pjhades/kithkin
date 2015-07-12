#include <kernel/buddy.h>

struct buddy_data buddydata;

void init_buddy(void)
{
    int order;

    for (order = 0; order <= MAX_ORDER; order++) {
        list_init(&buddydata.free_areas[order].pages);
        buddydata.free_areas[order].size = 0;
    }
}

static int get_buddy_idx(int page_idx, int order)
{
    return page_idx ^ (1 << order);
}

static int buddy_range_is_valid(int buddy_idx)
{
    int pfn = buddy_idx + minpfn;

    return pfn >= minpfn && pfn <= maxpfn;
}

void free_pages(struct page *victim, int order)
{
    struct page *page, *buddy;
    int start_idx, buddy_idx, combined_idx;

    page = victim;
    while (order <= MAX_ORDER) {
        start_idx = page_to_idx(page) & ~((1 << order) - 1);
        buddy_idx = get_buddy_idx(start_idx, order);

        if (!buddy_range_is_valid(buddy_idx))
            break;

        buddy = idx_to_page(buddy_idx);

        if (!(buddy->flags & (1 << PG_BUDDY)) || page_order(buddy) != order)
            break;

        list_remove(&buddy->lru);
        buddydata.free_areas[order].size--;

        set_page_order(buddy, 0);
        buddy->flags &= ~(1 << PG_BUDDY);

        combined_idx = start_idx & buddy_idx;
        page = idx_to_page(combined_idx);

        order++;
    }

    set_page_order(page, order);
    page->flags |= (1 << PG_BUDDY);
    list_insert_head(&page->lru, &buddydata.free_areas[order].pages);
    buddydata.free_areas[order].size++;
}

/*
 * Split page group of `order` into pieces so that we can only
 * use the first page group of `req_order`. Return the rest of
 * pages to the buddy system.
 */
static void split(struct page *page, int order, int req_order)
{
    struct page *half;

    while (order > req_order) {
        half = page + (1 << (order - 1));
        free_pages(half, order - 1);
        order--;
    }
}

/* TODO rough version, may need further fining work
 * flags, invoke oom, etc.
 */
struct page *alloc_pages(int order)
{
    struct page *page = NULL;
    int req_order = order;

    while (order <= MAX_ORDER) {
        if (buddydata.free_areas[order].size > 0) {
            list_get_head(page, &buddydata.free_areas[order].pages, lru);
            list_remove(&page->lru);
            buddydata.free_areas[order].size--;

            set_page_order(page, 0);
            page->flags &= ~(1 << PG_BUDDY);

            split(page, order, req_order);

            return page;
        }

        order++;
    }

    return page;
}
