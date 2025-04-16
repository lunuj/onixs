#ifndef IDE_H
#define IDE_H

#include "onixs/types.h"
#include "onixs/mutex.h"

#define SECTOR_SIZE 512

#define IDE_CTRL_NR 2
#define IDE_DISK_NR 2

typedef struct ide_disk_t
{
    char name[8];
    struct ide_ctrl_t * ctrl;
    uint8 selector;
    bool master;
}ide_disk_t;

typedef struct ide_ctrl_t
{
    char name[8];
    lock_t lock;
    uint16 iobase;
    ide_disk_t disk[IDE_DISK_NR];
    ide_disk_t * active;
} ide_ctrl_t;

void ide_init();
void ide_pio_read(ide_disk_t * disk, void * buf, uint8 count, idx_t lba);
void ide_pio_write(ide_disk_t * disk, void * buf, uint8 count, idx_t lba);
#endif // IDE_H