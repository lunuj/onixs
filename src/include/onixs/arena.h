#ifndef ARENA_H
#define ARENA_H

#include <onixs/types.h>
#include <onixs/list.h>

#define DESC_COUNT 7

typedef list_node_t block_t;

typedef struct arena_descriptor_t
{
    uint32 total_block;
    uint32 block_size;
    list_t free_list;
}arena_descriptor_t;

typedef struct arena_t
{
    arena_descriptor_t * desc;
    uint32 count;
    uint32 large;
    uint32 magic;
}arena_t;


void arena_init();
void * kmalloc(size_t size);
void kfree(void *ptr);
#endif // ARENA_H