#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <list.h>
#include <kernel/mm.h>

/* serves requests of up to 2^10 pages */
#define MAX_ORDER 10

struct free_area {
    struct list_node pages;
    int size;
};

struct buddy_data {
    struct free_area free_areas[MAX_ORDER];
};

void init_buddy(void);
void free_pages(struct page *ptr, int order);

#endif
