#ifndef GLOBAL_H
#define GLOBAL_H

#include <onixs/types.h>

#define KERNEL_CODE_IDX 1
#define KERNEL_DATA_IDX 2

#define KERNEL_CODE_SELECROT (KERNEL_CODE_IDX << 3)
#define KERNEL_DATA_SELECROT (KERNEL_DATA_IDX << 3)

#define GDT_SIZE 128

typedef struct descriptor_t
{
    uint16 limit_low;      // 段界限 0 ~ 15 位
    uint32 base_low : 24;  // 基地址 0 ~ 23 位 16M
    uint8 type : 4;        // 段类型
    uint8 segment : 1;     // 1 表示代码段或数据段，0 表示系统段
    uint8 DPL : 2;         // Descriptor Privilege Level 描述符特权等级 0 ~ 3
    uint8 present : 1;     // 存在位，1 在内存中，0 在磁盘上
    uint8 limit_high : 4;  // 段界限 16 ~ 19;
    uint8 available : 1;   // 该安排的都安排了，送给操作系统吧
    uint8 long_mode : 1;   // 64 位扩展标志
    uint8 big : 1;         // 32 位 还是 16 位;
    uint8 granularity : 1; // 粒度 4KB 或 1B
    uint8 base_high;       // 基地址 24 ~ 31 位
} _packed descriptor_t;

// 段选择子
typedef struct selector_t
{
    uint8 RPL : 2;
    uint8 TI : 1;
    uint16 index : 13;
} selector_t;

// 全局描述符表指针
typedef struct pointer_t
{
    uint16 limit;
    uint32 base;
} _packed pointer_t;

void gdt_init();
#endif // GLOBAL_H