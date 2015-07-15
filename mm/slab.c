#include <list.h>
#include <bitops.h>
#include <kernel/kernel.h>
#include <kernel/slab.h>
#include <kernel/buddy.h>

struct cache_sizes kmalloc_sizes[] = {
#define CACHE(x) { .name = "kmalloc-size-"#x, .size = x },
#include <kernel/kmalloc_sizes.h>
#undef CACHE
};

struct list_node slab_caches;

/* cache1: for allocating struct slab_cache
 * cache2: for allocating struct slab
 */
struct slab_cache cache1, cache2;

//static void init_other_slabs(void)
//{
//    struct slab_cache *cachep;
//    struct cache_sizes *sizep;
//
//    for (sizep = kmalloc_sizes; sizep->size > 0; sizep++) {
//        if (sizep->size == first_cache.objsize)
//            continue;
//
//        cachep = kmalloc();
//    }
//}

static void cache_grow(struct slab_cache *cache)
{
}

static int check_slab_size(u32 nr_obj, size_t objsize)
{
    return align(sizeof(struct slab) + nr_obj * sizeof(u32), WORD_SIZE) +
        nr_obj * objsize < PAGE_SIZE;
}

#define size_idx(size) (log2(size) - 2)

/* Manually initialize the caches for slab_cache and slab
 * descriptors in order to make kmalloc available for the
 * initialization of other sizes.
 */
static void init_ancestor_cache(struct slab_cache *cache, size_t realsize)
{
    size_t objsize;
    struct page *page;
    struct slab *slab;
    u32 nr_obj;
    int i;

    objsize = next_pow2_32bit(realsize);

    cache->name = kmalloc_sizes[size_idx(objsize)].name;
    cache->realsize = realsize;
    cache->objsize = objsize;

    list_init(&cache->list_free);
    list_init(&cache->list_partial);
    list_init(&cache->list_full);

    page = alloc_pages(0);
    if (!page)
        die("no enough memory to init ancestor caches\n");

    slab = (struct slab *)direct_map_page_to_virt(page);
    slab->cache = cache;

    nr_obj = PAGE_SIZE / objsize;
    while (!check_slab_size(nr_obj, objsize))
        nr_obj--;

    cache->objnum = nr_obj;
    slab->objs = (void *)align((u32)slab + sizeof(struct slab) +
            nr_obj * sizeof(u32), WORD_SIZE);
    slab->nr_inuse = 0;

    /* initialize the slab free array */
    slab->free = 0;
    for (i = 0; i < nr_obj - 1; i++)
        slab_freearray(slab)[i] = i + 1;
    slab_freearray(slab)[nr_obj - 1] = SLAB_FREEARRAY_END;

    list_insert_head(&slab->next, &cache->list_free);
    list_insert_tail(&cache->next, &slab_caches);

    kmalloc_sizes[size_idx(objsize)].cache = cache;
}

void init_slab(void)
{
    list_init(&slab_caches);

    init_ancestor_cache(&cache1, sizeof(struct slab_cache));
    init_ancestor_cache(&cache2, sizeof(struct slab));

    //init_other_slabs(void);
}

static int cache_is_empty(struct slab_cache *cache)
{
    return list_is_empty(&cache->list_free)
        && list_is_empty(&cache->list_partial);
}

void *kmalloc(size_t size)
{
    struct page *page;
    struct slab *slab;
    struct slab_cache *cache;
    int nr_pages, grown = 0;
    size_t p2size;
    void *obj;

    if (size <= 0)
        return NULL;

    if (size >= PAGE_SIZE) {
        nr_pages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
        page = alloc_pages(log2(nr_pages));
        if (!page)
            return NULL;
        return (void *)direct_map_page_to_virt(page);
    }

    p2size = next_pow2_32bit(size);
    cache = kmalloc_sizes[size_idx(p2size)].cache;

    if (!cache)
        die("kmalloc is not properly initialized\n");

    if (cache_is_empty(cache)) {
        cache_grow(cache);
        grown = 1;
    }

    if (grown)
        list_get_head(slab, &cache->list_free, next);
    else
        list_get_head(slab, &cache->list_partial, next);

    obj = (char *)slab->objs + slab->cache->objsize * slab->free;
    slab->free = slab_freearray(slab)[slab->free];
    slab->nr_inuse++;

    if (slab->nr_inuse == slab->cache->objnum) {
        list_remove(&slab->next);
        list_insert_head(&slab->next, &slab->cache->list_full);
    }

    return obj;
}

void kfree(void *obj)
{
    struct slab *slab;
    u32 idx;

    slab = (struct slab *)p2rounddown((u32)obj, PAGE_SIZE);

    idx = (u32)obj - (u32)slab->objs;
    slab_freearray(slab)[idx] = slab->free;
    slab->free = idx;
    slab->nr_inuse--;

    if (slab->nr_inuse == 0) {
        list_remove(&slab->next);
        list_insert_head(&slab->next, &slab->cache->list_free);
    }
}
