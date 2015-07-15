#ifndef __SLAB_H__
#define __SLAB_H__

#include <list.h>
#include <kernel/types.h>

struct slab_cache {
    char *name;
    struct list_node next;
    size_t objsize;
    size_t realsize;
    u32 objnum;
    struct list_node list_free;
    struct list_node list_partial;
    struct list_node list_full;
};

struct slab {
    void *objs;
    u32 free;
    u32 nr_inuse;
    struct slab_cache *cache;
    struct list_node next;
};

#define SLAB_FREEARRAY_END -1
#define slab_freearray(slab) ((char *)(slab) + sizeof(struct slab))

struct cache_sizes {
    char *name;
    size_t size;
    struct slab_cache *cache;
};

// TODO not implemented
struct slab_cache *slab_cache_create(const char *name, size_t size,
        size_t align, u32 flags);
void init_slab(void);
void *kmalloc(size_t size);
#endif
