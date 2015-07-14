#ifndef __SLAB_H__
#define __SLAB_H__

#include <list.h>
#include <kernel/types.h>

struct slab_cache {
    const char *name;
    struct list_node next;
    size_t objsize;
    size_t objnum;
    size_t realsize;
    u32 order;
    struct list_node list_free;
    struct list_node list_partial;
    struct list_node list_full;
};

struct slab {
    void *objs;
    u32 free;
    u32 nr_inuse;
    struct list_node list;
};

struct cache_sizes {
    const char *name;
    size_t size;
};

// TODO not implemented
struct slab_cache *slab_cache_create(const char *name, size_t size,
        size_t align, u32 flags);
void init_slab(void);
#endif
