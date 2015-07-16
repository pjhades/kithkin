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

#define size_idx(size) (log2(size) - 2)

struct list_node slab_caches;

/* cache1: for allocating struct slab_cache
 * cache2: for allocating struct slab
 */
struct slab_cache cache1, cache2;

static int check_slab_size(u32 nr_obj, size_t objsize)
{
    return align(sizeof(struct slab) + nr_obj * sizeof(u32), WORD_SIZE) +
        nr_obj * objsize < PAGE_SIZE;
}

static void cache_grow(struct slab_cache *cache)
{
    struct page *page;
    struct slab *slab;
    u32 nr_obj;
    int i;

    page = alloc_pages(0);
    if (!page)
        die("no enough memory to init ancestor caches\n");

    slab = (struct slab *)direct_map_page_to_virt(page);
    slab->cache = cache;

    nr_obj = PAGE_SIZE / cache->objsize;
    while (!check_slab_size(nr_obj, cache->objsize))
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
}

/* Create a new slab cache. If `cache` is NULL, allocate
 * a new one with kmalloc, otherwise we just fill the members
 * of `cache`.
 *
 * Return the created cache.
 */
static struct slab_cache *add_cache(struct slab_cache *cache, char *name,
        size_t realsize)
{
    struct slab_cache *c;
    size_t objsize = next_pow2_32bit(realsize);

    if (!cache) {
        cache = kmalloc(sizeof(struct slab_cache));
        if (!cache)
            die("cannot create kmalloc caches\n");
    }

    cache->name = name;
    cache->objsize = objsize;
    cache->realsize = realsize;

    list_init(&cache->list_free);
    list_init(&cache->list_partial);
    list_init(&cache->list_full);

    list_foreach(c, &slab_caches, next)
        if (c->objsize > cache->objsize)
            break;
    list_insert_before(&cache->next, &c->next);

    printk("create cache %s, objsize=%d, realsize=%d\n",
            cache->name, cache->objsize, cache->realsize);

    return cache;
}

static void add_kmalloc_caches(void)
{
    struct cache_sizes *sizep;
    struct slab_cache *cache;

    for (sizep = kmalloc_sizes; sizep->size > 0; sizep++) {
        if (sizep->cache)
            continue;

        cache = add_cache(NULL, sizep->name, sizep->size);
        kmalloc_sizes[size_idx(cache->objsize)].cache = cache;
    }
}

static int cache_is_empty(struct slab_cache *cache)
{
    return list_is_empty(&cache->list_free)
        && list_is_empty(&cache->list_partial);
}

void init_slab(void)
{
    size_t objsize;

    list_init(&slab_caches);

    /* Initialize the caches for slab_cache and slab
     * descriptors in order to make kmalloc available for the
     * initialization of other sizes.
     */
    objsize = next_pow2_32bit(sizeof(struct slab_cache));
    add_cache(&cache1, kmalloc_sizes[size_idx(objsize)].name, objsize);
    kmalloc_sizes[size_idx(cache1.objsize)].cache = &cache1;

    objsize = next_pow2_32bit(sizeof(struct slab));
    add_cache(&cache2, kmalloc_sizes[size_idx(objsize)].name, objsize);
    kmalloc_sizes[size_idx(cache2.objsize)].cache = &cache2;

    add_kmalloc_caches();
}

void *kmalloc(size_t size)
{
    struct page *page;
    struct slab *slab;
    struct slab_cache *cache;
    int nr_pages;
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

    if (cache_is_empty(cache))
        cache_grow(cache);

    if (!list_is_empty(&cache->list_partial))
        list_get_head(slab, &cache->list_partial, next);
    else
        list_get_head(slab, &cache->list_free, next);

    obj = (char *)slab->objs + slab->cache->objsize * slab->free;
    slab->free = slab_freearray(slab)[slab->free];
    slab->nr_inuse++;

    if (slab->nr_inuse == slab->cache->objnum) {
        list_remove(&slab->next);
        list_insert_head(&slab->next, &slab->cache->list_full);
    } else if (slab->nr_inuse == 1) {
        list_remove(&slab->next);
        list_insert_head(&slab->next, &slab->cache->list_partial);
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
    } else if (slab->nr_inuse == slab->cache->objnum - 1) {
        list_remove(&slab->next);
        list_insert_head(&slab->next, &slab->cache->list_partial);
    }
}

struct slab_cache *slab_cache_create(char *name, size_t size, size_t align,
        u32 flags)
{
    return add_cache(NULL, name, size);
}
