#ifndef BITMAP_H
#define BITMAP_H

#include <onixs/onixs.h>
#include <onixs/assert.h>
#include <onixs/string.h>
#include <onixs/debug.h>
#include <onixs/types.h>

typedef struct bitmap_t
{
    uint8 *bits;   // 位图缓冲区
    uint32 length; // 位图缓冲区长度
    uint32 offset; // 位图开始的偏移
} bitmap_t;

void bitmap_make(bitmap_t * map, char * bits, uint32 length, uint32 offset);
void bitmap_init(bitmap_t * map, char * bits, uint32 length, uint32 start);
bool bitmap_test(bitmap_t * map, idx_t index);
void bitmap_set(bitmap_t * map, idx_t index, bool value);
int bitmap_scan(bitmap_t * map, uint32 count);
void bitmap_tests();

#endif // BITMAP_H