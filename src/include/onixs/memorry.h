#ifndef MEMORRY_H
#define MEMORRY_H

#include <onixs/memorry.h>
#include <onixs/debug.h>
#include <onixs/assert.h>
#include <onixs/types.h>
#include <onixs/onixs.h>
#include <onixs/stdlib.h>
#include <onixs/string.h>
#include <onixs/bitmap.h>

#define ZONE_VALID 1
#define ZONE_RESERVED 2

#define MEMORY_PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置

#define KERNEL_PAGE_DIR 0x1000              //内核页目录 大小4KB
#define KERNEL_MAP_BITS 0x6000              //位图存放位置
// 内核占用的内存大小 16M
#define KERNEL_MEMORY_SIZE (0x100000 * 16)  //1M * 16
// 内核缓存地址
#define KERNEL_BUFFER_MEM 0x800000
// 内核缓存大小
#define KERNEL_BUFFER_SIZE 0x400000
// 内存虚拟磁盘地址
#define KERNEL_RAMDISK_MEM (KERNEL_BUFFER_MEM + KERNEL_BUFFER_SIZE)
// 内存虚拟磁盘大小
#define KERNEL_RAMDISK_SIZE 0x400000// 内核占用的内存大小 16M
// 内核缓存地址
#define KERNEL_BUFFER_MEM 0x800000
// 内核缓存大小
#define KERNEL_BUFFER_SIZE 0x400000
// 内存虚拟磁盘地址
#define KERNEL_RAMDISK_MEM (KERNEL_BUFFER_MEM + KERNEL_BUFFER_SIZE)
// 内存虚拟磁盘大小
#define KERNEL_RAMDISK_SIZE 0x400000
// 用户程序地址
#define USER_EXEC_ADDR KERNEL_MEMORY_SIZE
// 用户映射内存开始位置
#define USER_MMAP_ADDR 0x8000000
// 用户映射内存大小
#define USER_MMAP_SIZE 0x8000000
// 用户栈顶地址
#define USER_STACK_TOP 0x10000000
// 用户栈大小
#define USER_STACK_SIZE 0x200000
// 用户栈底地址
#define USER_STACK_BOTTOM (USER_STACK_TOP - USER_STACK_SIZE)

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
    uint8 shared : 1;   // 共享内存页 CPU无关
    uint8 privat : 1;   // 私有内存页 CPU无关
    uint8 flag : 1;     // 
    uint32 index : 20;  // 页索引
}_packed page_entry_t;

uint32 get_cr2();
uint32 get_cr3();
void set_cr3(uint32 pde);
void memory_map_init();
void mapping_init();
uint32 alloc_kpage(uint32 count);
void free_kpage(uint32 vaddr, uint32 count);

void link_page(uint32 vaddr);
void unlink_page(uint32 vaddr);
page_entry_t * copy_pde();
void free_pde();
void page_fault(
    int vector,
    uint32 edi, uint32 esi, uint32 ebp, uint32 esp,
    uint32 ebx, uint32 edx, uint32 ecx, uint32 eax,
    uint32 gs, uint32 fs, uint32 es, uint32 ds,
    uint32 vector0, uint32 error, uint32 eip, uint32 cs, uint32 eflags);
page_entry_t *get_entry(uint32 vaddr, bool create);
void flush_tlb(uint32 vaddr);
void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int sys_munmap(void *addr, size_t length);
// 系统调用相关
int32 sys_brk(void * addr);
#endif // MEMORRY_H