#include "mem_block.h"
#include "alloc.h"

#define MAX_CACHE_COUNT             128
#define MAX_GATHER_MEM              MAX_FRAME_SIZE

static struct list_head cache_heads;
static LOCK_T cache_lock;
static int32_t cache_counts = 0;


void
init_memblock_facility( void )
{
    INIT_LIST_HEAD(&cache_heads);
    cache_lock = LOCK_NEW();    
}


static __inline__ mem_block*
__alloc_memblock( void )
{
    struct list_head *l;

    if (cache_counts > 0)
    {
        l = cache_heads.next;
        list_del(l);
        --cache_counts;
        return container_of(l, mem_block, list);
    }

    return (mem_block*)tr_alloc(sizeof(mem_block));
}


mem_block *
alloc_memblock( void )
{
    mem_block *mb;

    AQUIRE_LOCK(cache_lock);
    mb = __alloc_memblock();
    RELEASE_LOCK(cache_lock);

    return mb;
}


static __inline__ void
__free_memblock(mem_block *mb)
{
    if (cache_counts < MAX_CACHE_COUNT)
    {
        list_add(&mb->list, &cache_heads);
        ++cache_counts;
        return;
    }

    tr_free(mb, sizeof(*mb));
}


static __inline__ void
_free_memblock(mem_block *mb)
{
    AQUIRE_LOCK(cache_lock);
    __free_memblock(mb);
    RELEASE_LOCK(cache_lock);
}


void
free_memblock(mem_block *mb)
{
    (*mb->finalize)(mb);
    _free_memblock(mb);
}


static void
fin_gather_memb_block(mem_block *mb)
{
    tr_free(mb->ptr, mb->b_size);
}


mem_block *
alloc_gather_memblock(uint32_t size)
{
    uint8_t *ptr;
    mem_block *mb;

    if (size > MAX_GATHER_MEM)
        return NULL;

    ptr = tr_alloc(size);
    if (ptr)
    {
        mb = alloc_memblock();
        if (mb)
        {
            INIT_LIST_HEAD(&mb->list);
            mb->ptr = ptr;
            mb->seq = 0;
            mb->offset = 0;
            mb->size = 0;
            mb->b_size = size;
            mb->u = NULL;
            mb->finalize = fin_gather_memb_block;
            return mb;
        }

        tr_free(ptr, size);
    }

    return NULL;
}


mem_block *
gather_memblock(mem_block *gather, mem_block *mb)
{
    uint32_t size, left;

    size = mb->size - mb->offset;
    left = gather->b_size - gather->size;

    if (size <= left)
    {
        memcpy(&gather->ptr[gather->size], &mb->ptr[mb->offset], size);
        gather->size += size;
        gather->seq = mb->seq;
        return gather;
    }

    return NULL;
}


//:~ End
