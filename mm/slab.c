#include <list.h>
#include <bitops.h>
#include <kernel/kernel.h>
#include <kernel/slab.h>

struct cache_sizes kmalloc_sizes[] = {
#define CACHE(x) { .name = "kmalloc-size-"#x, .size = x },
#include <kernel/kmalloc_sizes.h>
#undef CACHE
};

struct list_node caches;
struct slab_cache first_cache;

void init_slab(void)
{
    struct cache_sizes *sizep;
    size_t size;

    list_init(&caches);

    size = next_pow2_32bit(sizeof(struct slab_cache));

    first_cache.name = kmalloc_sizes[size].name;
    first_cache.realsize = kmalloc_sizes[size].size;

    list_init(&first_cache.list_free);
    list_init(&first_cache.list_partial);
    list_init(&first_cache.list_full);
}
