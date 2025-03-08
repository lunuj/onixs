#ifndef MEMORRY_H
#define MEMORRY_H

#include <onixs/memorry.h>
#include <onixs/debug.h>
#include <onixs/assert.h>
#include <onixs/types.h>
#include <onixs/onixs.h>
#include <onixs/stdlib.h>
#include <onixs/string.h>

#define ZONE_VALID 1
#define ZONE_RESERVED 2

#define PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置

#define IDX(addr) ((uint32)addr >> 12)
#define PAGE(idx) ((uint32)idx << 12)
#define ASSERT_PAGE(addr) assert((addr & 0xFFF) == 0)

typedef struct ards_t
{
    uint64 base;
    uint64 size;
    uint32 type;
}_packed ards_t;

void memory_map_init();
void memory_test();


#endif // MEMORRY_H