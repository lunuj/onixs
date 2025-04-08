#include <onixs/arena.h>
#include <onixs/memorry.h>
#include <onixs/string.h>
#include <onixs/stdlib.h>
#include <onixs/assert.h>

static arena_descriptor_t descriptor[DESC_COUNT];

void arena_init()
{
    uint32 block_size = 16;
    for(size_t i = 0; i < DESC_COUNT; i++)
    {
        arena_descriptor_t * desc = &descriptor[i];
        desc->block_size = block_size;
        desc->total_block = (MEMORY_PAGE_SIZE - sizeof(arena_t)) / block_size;
        list_init(&desc->free_list);
        block_size <<= 1;
    }
}

static void * arena_get_block(arena_t * arena, uint32 idx)
{
    assert(arena->desc->total_block > idx);
    void * addr = (void *)(arena+1);
    uint32 gap = idx * arena->desc->block_size;
    return addr + gap;
}

static arena_t * arena_get_arena(block_t * block)
{
    return (arena_t *)((uint32)block & 0xFFFFF000);
}

void * kmalloc(size_t size)
{
    arena_descriptor_t * desc = NULL;
    arena_t * arena;
    block_t * block;
    char * addr;
    if(size > 1024)
    {
        uint32 asize = size + sizeof(arena_t);
        uint32 count = div_round_up(asize, MEMORY_PAGE_SIZE);

        arena = (arena_t *)alloc_kpage(count);
        memset(arena, 0,  count * MEMORY_PAGE_SIZE);
        arena->large = true;
        arena->count = count;
        arena->desc = NULL;
        arena->magic = ONIXS_MAGIC;

        addr = (char *)((uint32)arena + sizeof(arena_t));
        return addr;
    }

    for(size_t i = 0; i < DESC_COUNT; i++)
    {
        desc = &descriptor[i];
        if(desc->block_size >= size)
            break;
    }

    assert(desc!=NULL);

    if(list_empty(&desc->free_list))
    {
        arena = (arena_t *)alloc_kpage(1);
        memset(arena, 0, MEMORY_PAGE_SIZE);

        arena->desc = desc;
        arena->large = false;
        arena->count = desc->total_block;
        arena->magic = ONIXS_MAGIC;

        for(size_t i = 0; i < desc->total_block; i++)
        {
            block = arena_get_block(arena, i);
            assert(!list_search(&arena->desc->free_list, block));
            list_push(&arena->desc->free_list, block);
            assert(list_search(&arena->desc->free_list, block));
        }
    }

    block = list_pop(&desc->free_list);
    arena = arena_get_arena(block);
    assert(arena->magic == ONIXS_MAGIC && !arena->large);

    arena->count--;
    return block;
}

void kfree(void *ptr)
{
    assert(ptr);

    block_t * block = (block_t *)ptr;
    arena_t * arena = arena_get_arena(block);

    assert(arena->large == 1 || arena->large == 0);
    assert(arena->magic == ONIXS_MAGIC);

    if(arena->large)
    {
        free_kpage((uint32)arena, arena->count);
        return;
    }

    list_push(&arena->desc->free_list, block);
    arena->count++;

    if(arena->count == arena->desc->total_block)
    {
        for(size_t i = 0; i <arena->desc->total_block; i++)
        {
            block = arena_get_block(arena, i);
            assert(list_search(&arena->desc->free_list, block));
            list_remove(block);
            assert(!list_search(&arena->desc->free_list, block));
        }
        free_kpage((uint32)arena, 1);
    }
}