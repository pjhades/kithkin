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

struct list_node caches;

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
//
//static void cache_grow(struct slab_cache *cache)
//{
//    struct slab *slab;
//
//    slab = 
//}

static int check_slab_size(u32 nr_obj, size_t objsize)
{
    return align(sizeof(struct slab) + nr_obj * sizeof(u32), WORD_SIZE) +
        nr_obj * objsize < PAGE_SIZE;
}

#define size_idx(size) (log2(size) - 2)

static void init_ancestor_cache(struct slab_cache *cache, size_t realsize)
{
    size_t objsize;
    struct page *page;
    struct slab *slab;
    u32 nr_obj;

    objsize = next_pow2_32bit(realsize);

    cache->name = kmalloc_sizes[size_idx(objsize)].name;
    cache->realsize = realsize;
    cache->objsize = objsize;

    list_init(&cache->list_free);
    list_init(&cache->list_partial);
    list_init(&cache->list_full);

    page = alloc_pages(0);
    slab = (struct slab *)direct_map_page_to_virt(page);
    printk("--> slab=%p, sizeof slab=%d\n", slab, sizeof(struct slab));

    nr_obj = PAGE_SIZE / objsize;
    while (!check_slab_size(nr_obj, objsize))
        nr_obj--;

    printk("--> nr_obj=%d\n", nr_obj);
    printk("--> to align=%d %p\n",
            sizeof(struct slab) + nr_obj * sizeof(u32),
            sizeof(struct slab) + nr_obj * sizeof(u32));
    printk("--> align=%d %p\n",
            align(sizeof(struct slab) + nr_obj * sizeof(u32), WORD_SIZE),
            align(sizeof(struct slab) + nr_obj * sizeof(u32), WORD_SIZE));
    cache->objnum = nr_obj;
    slab->objs = (void *)align((u32)slab + sizeof(struct slab) +
            nr_obj * sizeof(u32), WORD_SIZE);
    printk("--> obj=%p\n", slab->objs);
    slab->nr_inuse = 0;
    slab->free = 0;
    list_insert_head(&slab->list, &cache->list_free);
}

void init_slab(void)
{
    //struct cache_sizes *sizep;

    list_init(&caches);

    init_ancestor_cache(&cache1, sizeof(struct slab_cache));
    init_ancestor_cache(&cache2, sizeof(struct slab));
    struct slab *slab;
    printk("cache1:\n");
    printk("  name: %s\n", cache1.name);
    printk("  objsize: %d\n", cache1.objsize);
    printk("  realsize: %d\n", cache1.realsize);
    printk("  objnum: %d\n", cache1.objnum);
    printk("  slab:\n");
    list_get_head(slab, &cache1.list_free, list);
    printk("    objs: %p\n", slab->objs);
    printk("    free: %p\n", slab->free);
    printk("    nr_inuse: %p\n", slab->nr_inuse);

    printk("cache2:\n");
    printk("  name: %s\n", cache2.name);
    printk("  objsize: %d\n", cache2.objsize);
    printk("  realsize: %d\n", cache2.realsize);
    printk("  objnum: %d\n", cache2.objnum);
    printk("  slab:\n");
    list_get_head(slab, &cache2.list_free, list);
    printk("    objs: %p\n", slab->objs);
    printk("    free: %p\n", slab->free);
    printk("    nr_inuse: %p\n", slab->nr_inuse);

    //init_other_slabs(void);
}
