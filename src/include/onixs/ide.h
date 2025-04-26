#ifndef IDE_H
#define IDE_H

#include "onixs/types.h"
#include "onixs/mutex.h"

#define SECTOR_SIZE 512

#define IDE_CTRL_NR 2
#define IDE_DISK_NR 2
#define IDE_PART_NR 4 // 每个磁盘分区数量，只支持主分区，总共 4 个

typedef struct part_entry_t
{
    uint8 bootable;             // 引导标志
    uint8 start_head;           // 分区起始磁头号
    uint8 start_sector : 6;     // 分区起始扇区号
    uint32 start_cylinder : 10; // 分区起始柱面号
    uint8 system;               // 分区类型字节
    uint8 end_head;             // 分区的结束磁头号
    uint8 end_sector : 6;       // 分区结束扇区号
    uint32 end_cylinder : 10;   // 分区结束柱面号
    uint32 start;               // 分区起始物理扇区号 LBA
    uint32 count;               // 分区占用的扇区数
} _packed part_entry_t;

typedef struct boot_sector_t
{
    uint8 code[446];
    part_entry_t entry[4];
    uint32 signature;
} _packed boot_sector_t;

typedef struct ide_part_t
{
    char name[8];            // 分区名称
    struct ide_disk_t *disk; // 磁盘指针
    uint32 system;              // 分区类型
    uint32 start;               // 分区起始物理扇区号 LBA
    uint32 count;               // 分区占用的扇区数
} ide_part_t;

typedef struct ide_disk_t
{
    char name[8];
    struct ide_ctrl_t * ctrl;
    uint8 selector;
    bool master;
    uint32 total_lba;
    uint32 cylinders;
    uint32 heads;
    uint32 sectors;
    ide_part_t parts[IDE_PART_NR]; // 硬盘分区
}ide_disk_t;

typedef struct ide_ctrl_t
{
    char name[8];
    lock_t lock;
    uint16 iobase;
    ide_disk_t disk[IDE_DISK_NR];
    ide_disk_t * active;
    uint8 control;
    struct task_t *waiter;
} ide_ctrl_t;

void ide_init();
int ide_pio_read(ide_disk_t * disk, void * buf, uint8 count, idx_t lba);
int ide_pio_write(ide_disk_t * disk, void * buf, uint8 count, idx_t lba);
#endif // IDE_H