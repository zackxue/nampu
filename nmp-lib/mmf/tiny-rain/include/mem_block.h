/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_MEM_BLOCK_H__
#define __TINY_RAIN_MEM_BLOCK_H__

#include "def.h"
#include "list.h"

BEGIN_NAMESPACE

#define MEMBLOCK_FLGS_DROPABLE	0x0001
#define MEMBLOCK_SET_SEQ(mb, _seq) ((mb)->seq = _seq)
#define MEMBLOCK_GET_SEQ(mb) ((mb)->seq)
#define MEMBLOCK_DROPABLE(mb) ((mb)->flags&MEMBLOCK_FLGS_DROPABLE)

typedef struct __mem_block mem_block;
struct __mem_block
{
    struct list_head list;
    uint8_t *ptr;
    uint32_t seq;
    uint32_t flags;
    uint32_t offset;
    uint32_t size;
    uint32_t b_size;    /* mem real size */
    void *u;
    void (*finalize)(mem_block *mb);
};

void init_memblock_facility( void );
mem_block *alloc_memblock( void );
void free_memblock(mem_block *mb);

mem_block *alloc_gather_memblock(uint32_t size);
mem_block *gather_memblock(mem_block *gather, mem_block *mb);

END_NAMESPACE

#endif  //__TINY_RAIN_MEM_BLOCK_H__
