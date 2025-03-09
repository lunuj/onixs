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

#define MEMORY_PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置

#define KERNEL_PAGE_DIR 0x1000            //内核页目录 大小4KB
#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABEL))

#define IDX(addr) ((uint32)addr >> 12)
#define DIDX(addr) (((uint32)addr >> 22) & 0x3FF)
#define TIDX(addr) (((uint32)addr >> 12) & 0x3FF)
#define PAGE(idx) ((uint32)idx << 12)
#define ASSERT_PAGE(addr) assert((addr & 0xFFF) == 0)

typedef struct ards_t
{
    uint64 base;
    uint64 size;
    uint32 type;
}_packed ards_t;

typedef struct page_entry_t{
    uint8 present : 1;  // 在内存中
    uint8 write : 1;    // 0 只读 1 可读可写
    uint8 user : 1;     // 1 所有人 0 超级用户 DPL < 3
    uint8 pwt : 1;      // page write through 1 直写模式，0 回写模式
    uint8 pcd : 1;      // page cache disable 禁止该页缓冲
    uint8 accessed : 1; // 被访问过，用于统计使用频率
    uint8 dirty : 1;    // 脏页，表示该页缓冲被写过
    uint8 pat : 1;      // page attribute table 页大小 4K/4M
    uint8 global : 1;   // 全局，所有进程都用到了，该页不刷新缓冲
    uint8 ignored : 3;  // 该安排的都安排了，送给操作系统吧
    uint32 index : 20;  // 页索引
}_packed page_entry_t;

uint32 get_cr3();
void set_cr3(uint32 pde);
void memory_map_init();
void mapping_init();
void memory_test();


#endif // MEMORRY_H