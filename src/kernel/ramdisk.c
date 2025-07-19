#include <onixs/memorry.h>
#include <onixs/debug.h>
#include <onixs/string.h>
#include <onixs/stdio.h>
#include <onixs/device.h>

#define SECTOR_SIZE 512

#define RAMDISK_NR  4

typedef struct ramdisk_t
{
    uint8 *start;   // 内存开始位置
    uint32 size;    // 内存占用大小
}ramdisk_t;

static ramdisk_t ramdisks[RAMDISK_NR];


int ramdisk_ioctl(ramdisk_t *disk, int cmd, void *args, int flags)
{
    switch(cmd)
    {
        case DEV_CMD_SECTOR_START:
            return 0;
        case DEV_CMD_SECTOR_COUNT:
            return disk->size / SECTOR_SIZE;
        default:
            panic("device command %d can't recognize", cmd);
            break;
    }
}

int ramdisk_read(ramdisk_t *disk, void *buf, uint8 count, idx_t lba)
{
    void *addr = disk->start + lba * SECTOR_SIZE;
    uint32 len = count * SECTOR_SIZE;
    assert((uint32)addr + len < (KERNEL_RAMDISK_MEM + KERNEL_RAMDISK_SIZE));
    memcpy(buf, addr, len);
    return count;
}

int ramdisk_write(ramdisk_t *disk, void *buf, uint8 count, idx_t lba)
{
    void *addr = disk->start + lba * SECTOR_SIZE;
    uint32 len = count * SECTOR_SIZE;
    assert((uint32)addr + len < (KERNEL_RAMDISK_MEM + KERNEL_RAMDISK_SIZE));
    memcpy(addr, buf, len);
    return count;
}

int ramdisk_init()
{
    SYS_LOG(LOG_INFO, "ramdisk init");

    uint32 size = KERNEL_RAMDISK_SIZE / RAMDISK_NR;
    assert(size % SECTOR_SIZE == 0);
    
    char name[32];

    for (size_t i = 0; i < RAMDISK_NR; i++)
    {
        ramdisk_t *ramdisk = &ramdisk[i];
        ramdisk->size = size;
        ramdisk->start = (uint8 *)(KERNEL_RAMDISK_MEM + size * i);
        sprintf(name, "md%c", i + 'a');
        device_install(DEV_BLOCK, DEV_RAMDISK, ramdisk, name, 0,
                        ramdisk_ioctl, ramdisk_read, ramdisk_write);
    }
}