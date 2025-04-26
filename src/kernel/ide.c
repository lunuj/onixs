#include <onixs/ide.h>
#include <onixs/debug.h>
#include <onixs/stdio.h>
#include <onixs/memorry.h>
#include <onixs/io.h>
#include <onixs/interrupt.h>
#include <onixs/task.h>
// IDE 寄存器基址
#define IDE_IOBASE_PRIMARY 0x1F0   // 主通道基地址
#define IDE_IOBASE_SECONDARY 0x170 // 从通道基地址

// IDE 寄存器偏移
#define IDE_DATA 0x0000       // 数据寄存器
#define IDE_ERR 0x0001        // 错误寄存器
#define IDE_FEATURE 0x0001    // 功能寄存器
#define IDE_SECTOR 0x0002     // 扇区数量
#define IDE_LBA_LOW 0x0003    // LBA 低字节
#define IDE_LBA_MID 0x0004    // LBA 中字节
#define IDE_LBA_HIGH 0x0005   // LBA 高字节
#define IDE_HDDEVSEL 0x0006   // 磁盘选择寄存器
#define IDE_STATUS 0x0007     // 状态寄存器
#define IDE_COMMAND 0x0007    // 命令寄存器
#define IDE_ALT_STATUS 0x0206 // 备用状态寄存器
#define IDE_CONTROL 0x0206    // 设备控制寄存器
#define IDE_DEVCTRL 0x0206    // 驱动器地址寄存器

// IDE 命令
#define IDE_CMD_READ 0x20     // 读命令
#define IDE_CMD_WRITE 0x30    // 写命令
#define IDE_CMD_IDENTIFY 0xEC // 识别命令

// IDE 控制器状态寄存器
#define IDE_SR_NULL 0x00 // NULL
#define IDE_SR_ERR 0x01  // Error
#define IDE_SR_IDX 0x02  // Index
#define IDE_SR_CORR 0x04 // Corrected data
#define IDE_SR_DRQ 0x08  // Data request
#define IDE_SR_DSC 0x10  // Drive seek complete
#define IDE_SR_DWF 0x20  // Drive write fault
#define IDE_SR_DRDY 0x40 // Drive ready
#define IDE_SR_BSY 0x80  // Controller busy

// IDE 控制寄存器
#define IDE_CTRL_HD15 0x00 // Use 4 bits for head (not used, was 0x08)
#define IDE_CTRL_SRST 0x04 // Soft reset
#define IDE_CTRL_NIEN 0x02 // Disable interrupts

// IDE 错误寄存器
#define IDE_ER_AMNF 0x01  // Address mark not found
#define IDE_ER_TK0NF 0x02 // Track 0 not found
#define IDE_ER_ABRT 0x04  // Abort
#define IDE_ER_MCR 0x08   // Media change requested
#define IDE_ER_IDNF 0x10  // Sector id not found
#define IDE_ER_MC 0x20    // Media change
#define IDE_ER_UNC 0x40   // Uncorrectable data error
#define IDE_ER_BBK 0x80   // Bad block

#define IDE_LBA_MASTER 0b11100000 // 主盘 LBA
#define IDE_LBA_SLAVE 0b11110000  // 从盘 LBA

// 分区文件系统
// 参考 https://www.win.tue.nl/~aeb/partitions/partition_types-1.html
typedef enum PART_FS
{
    PART_FS_FAT12 = 1,    // FAT12
    PART_FS_EXTENDED = 5, // 扩展分区
    PART_FS_MINIX = 0x80, // minux
    PART_FS_LINUX = 0x83, // linux
} PART_FS;

typedef struct ide_params_t
{
    uint16 config;                 // 0 General configuration bits
    uint16 cylinders;              // 01 cylinders
    uint16 RESERVED;               // 02
    uint16 heads;                  // 03 heads
    uint16 RESERVED[5 - 3];        // 05
    uint16 sectors;                // 06 sectors per track
    uint16 RESERVED[9 - 6];        // 09
    uint8 serial[20];              // 10 ~ 19 序列号
    uint16 RESERVED[22 - 19];      // 10 ~ 22
    uint8 firmware[8];             // 23 ~ 26 固件版本
    uint8 model[40];               // 27 ~ 46 模型数
    uint8 drq_sectors;             // 47 扇区数量
    uint8 RESERVED[3];             // 48
    uint16 capabilities;           // 49 能力
    uint16 RESERVED[59 - 49];      // 50 ~ 59
    uint32 total_lba;              // 60 ~ 61
    uint16 RESERVED;               // 62
    uint16 mdma_mode;              // 63
    uint8 RESERVED;                // 64
    uint8 pio_mode;                // 64
    uint16 RESERVED[79 - 64];      // 65 ~ 79 参见 ATA specification
    uint16 major_version;          // 80 主版本
    uint16 minor_version;          // 81 副版本
    uint16 commmand_sets[87 - 81]; // 82 ~ 87 支持的命令集
    uint16 RESERVED[118 - 87];     // 88 ~ 118
    uint16 support_settings;       // 119
    uint16 enable_settings;        // 120
    uint16 RESERVED[221 - 120];    // 221
    uint16 transport_major;        // 222
    uint16 transport_minor;        // 223
    uint16 RESERVED[254 - 223];    // 254
    uint16 integrity;              // 校验和
} _packed ide_params_t;

ide_ctrl_t controllers[IDE_CTRL_NR];

/**
 * @brief  输出ctrl的错误状态
 * @param  *ctrl IDE控制器
 * @retval 
 * @note 
 */
static uint32 ide_error(ide_ctrl_t *ctrl)
{
    uint8 error = inb(ctrl->iobase + IDE_ERR);
    if (error & IDE_ER_BBK)
        LOGK("bad block\n");
    if (error & IDE_ER_UNC)
        LOGK("uncorrectable data\n");
    if (error & IDE_ER_MC)
        LOGK("media change\n");
    if (error & IDE_ER_IDNF)
        LOGK("id not found\n");
    if (error & IDE_ER_MCR)
        LOGK("media change requested\n");
    if (error & IDE_ER_ABRT)
        LOGK("abort\n");
    if (error & IDE_ER_TK0NF)
        LOGK("track 0 not found\n");
    if (error & IDE_ER_AMNF)
        LOGK("address mark not found\n");
    return error;
}

/**
 * @brief  等待ctrl控制器的mask状态
 * @param  *ctrl 控制
 * @param  mask 状态
 * @retval 
 * @note 
 */
static void ide_busy_wait(ide_ctrl_t * ctrl, uint8 mask)
{
    while(true)
    {
        uint8 state = inb(ctrl->iobase + IDE_ALT_STATUS);
        if(state & IDE_SR_ERR)
            ide_error(ctrl);
        if(state & IDE_SR_BSY)
            continue;
        if((state & mask) == mask)
            return;
    }
}

/**
 * @brief  控制器选择硬盘disk
 * @param  *disk 硬盘
 * @retval 无
 * @note 
 */
static void ide_select_drive(ide_disk_t *disk)
{
    outb(disk->ctrl->iobase + IDE_HDDEVSEL, disk->selector);
    disk->ctrl->active = disk;
}

static void ide_select_sector(ide_disk_t *disk, uint32 lba, uint8 count)
{
    outb(disk->ctrl->iobase + IDE_FEATURE, 0);
    outb(disk->ctrl->iobase + IDE_SECTOR, count);
    outb(disk->ctrl->iobase + IDE_LBA_LOW, lba & 0xFF);
    outb(disk->ctrl->iobase + IDE_LBA_MID, (lba & 0xFF00) >> 8);
    outb(disk->ctrl->iobase + IDE_LBA_HIGH, (lba & 0xFF0000)>>16);
    outb(disk->ctrl->iobase + IDE_HDDEVSEL, ((lba >> 24) & 0xF) | disk->selector);
}

static void ide_reset_control(ide_ctrl_t * ctrl)
{
    outb(ctrl->iobase + IDE_CONTROL, IDE_CTRL_SRST);
    ide_busy_wait(ctrl, IDE_SR_NULL);
    outb(ctrl->iobase + IDE_CONTROL, ctrl->control);
    ide_busy_wait(ctrl, IDE_SR_NULL);
}

static void ide_pio_read_sector(ide_disk_t * disk, uint16 * buf)
{
    for(size_t i = 0; i < (SECTOR_SIZE / 2); i++)
    {
        buf[i] = inw(disk->ctrl->iobase + IDE_DATA);
    }
}

static void ide_swap_pairs(char *buf, uint32 len)
{
    for(size_t i = 0; i < len; i+=2)
    {
        char tmp = buf[i];
        buf[i] = buf[i+1];
        buf[i + 1] = tmp;
    }
    buf[len - 1] = '\0';
}

static uint32 ide_identify(ide_disk_t * disk, uint16 * buf)
{
    lock_acquire(&disk->ctrl->lock);
    ide_select_drive(disk);

    outb(disk->ctrl->iobase + IDE_COMMAND, IDE_CMD_IDENTIFY);
    ide_busy_wait(disk->ctrl, IDE_SR_NULL);
    ide_params_t * params = (ide_params_t *)buf;
    ide_pio_read_sector(disk, buf);
    LOGK("[INFO]: disk %s total lba %d\n", disk->name, params->total_lba);

    uint32 ret = EOF;
    if(params->total_lba == 0)
    {
        goto rollback;
    }

    ide_swap_pairs(params->serial, sizeof(params->serial));
    LOGK("[INFO]: disk %s total lba %s\n", disk->name, params->serial);

    ide_swap_pairs(params->firmware, sizeof(params->firmware));
    LOGK("[INFO]: disk %s firware %s\n", disk->name, params->firmware);

    ide_swap_pairs(params->model, sizeof(params->model));
    LOGK("[INFO]: disk %s model %s\n", disk->name, params->model);

    disk->total_lba = params->total_lba;
    disk->cylinders = params->cylinders;
    disk->heads = params->heads;
    disk->sectors = params->sectors;
    ret = 0;
rollback:
    lock_release(&disk->ctrl->lock);
    return ret;
}

/**
 * @brief  从disk中的lba块读取count个扇区到buf中
 * @param  disk 要读取的硬盘
 * @param  buf 存储读取数据的缓冲
 * @param  count 读取扇区的数量
 * @param  lba 逻辑块地址
 * @retval int 
 * @note 
 */
int ide_pio_read(ide_disk_t * disk, void * buf, uint8 count, idx_t lba)
{
    assert(count > 0);
    assert(!interrupt_get_state());
    ide_ctrl_t * ctrl = disk->ctrl;

    lock_acquire(&ctrl->lock);
    ide_select_drive(disk);
    ide_busy_wait(ctrl, IDE_SR_DRDY);
    ide_select_sector(disk, lba, count);
    outb(ctrl->iobase + IDE_COMMAND, IDE_CMD_READ);

    for(size_t i = 0; i < count; i++)
    {
        task_t * task = running_task();
        if(task->state == TASK_RUNNING)
        {
            ctrl->waiter = task;
            task_block(task, NULL, TASK_BLOCKED);
        }
        ide_busy_wait(ctrl, IDE_SR_DRQ);
        uint32 offset = ((uint32)buf + i * SECTOR_SIZE);
        ide_pio_read_sector(disk, (uint16 *)offset);
    }
    lock_release(&ctrl->lock);
    return 0;
}

int ide_pio_part_read(ide_part_t *part, void *buf, uint8 count, idx_t lba)
{
    return ide_pio_read(part->disk, buf, count, part->start + lba);
}

static void ide_pio_write_sector(ide_disk_t * disk, uint16 * buf)
{
    for(size_t i = 0; i < (SECTOR_SIZE / 2); i++)
    {
        outw(disk->ctrl->iobase + IDE_DATA, buf[i]);
    }
}

int ide_pio_write(ide_disk_t * disk, void * buf, uint8 count, idx_t lba)
{
    assert(count > 0);
    assert(!interrupt_get_state());
    ide_ctrl_t * ctrl = disk->ctrl;
    lock_acquire(&ctrl->lock);

    ide_select_drive(disk);
    ide_busy_wait(ctrl, IDE_SR_DRDY);
    ide_select_sector(disk, lba, count);

    outw(ctrl->iobase + IDE_COMMAND, IDE_CMD_WRITE);
    for(size_t i = 0; i < count;i++)
    {
        uint32 offset = ((uint32)buf + i * SECTOR_SIZE);
        ide_pio_write_sector(disk, (uint16 *)offset);
        task_t * task = running_task();
        if(task->state == TASK_RUNNING)
        {
            ctrl->waiter = task;
            task_block(task, NULL, TASK_BLOCKED);
        }
        ide_busy_wait(ctrl, IDE_SR_NULL);
    }
    lock_release(&ctrl->lock);
    return 0;
}

int ide_pio_part_write(ide_part_t *part, void *buf, uint8 count, idx_t lba)
{
    return ide_pio_write(part->disk, buf, count, part->start + lba);
}

void ide_handler(int vector)
{
    send_eoi(vector);
    ide_ctrl_t * ctrl = &controllers[vector - IRQ_HARDDISK - 0x20];
    uint8 state = inb(ctrl->iobase + IDE_STATUS);
    LOGK("[INFO]: state %d %#x", vector, state);
    if(ctrl->waiter)
    {
        task_unblock(ctrl->waiter);
        ctrl->waiter == NULL;
    }
}

static void ide_part_init(ide_disk_t *disk, uint16 *buf)
{
    if(!disk->total_lba)
        return;
    ide_pio_read(disk,buf, 1, 0);
    boot_sector_t *boot = (boot_sector_t *)buf;

    for(size_t i = 0; i < IDE_PART_NR; i++)
    {
        part_entry_t *entry = &boot->entry[i];
        ide_part_t *part = &disk->parts[i];
        if(!entry->count)
            continue;
        
        sprintf(part->name, "%s%d", disk->name, i+1);

        LOGK("part %s \n", part->name);
        LOGK("    bootable %d\n", entry->bootable);
        LOGK("    start %d\n", entry->start);
        LOGK("    count %d\n", entry->count);
        LOGK("    system %#x\n", entry->system);

        part->disk = disk;
        part->count = entry->count;
        part->system = entry->system;
        part->start = entry->start;

        if(entry->system == PART_FS_EXTENDED)
        {
            LOGK("[INFO]: unsupported extended partition\n");

            boot_sector_t *eboot = (boot_sector_t *)(buf + SECTOR_SIZE);
            ide_pio_read(disk, (void *)eboot, 1, entry->start);

            for(size_t j = 0; j < IDE_PART_NR; j++)
            {
                part_entry_t *eentry = &eboot->entry[j];
                if(!eentry->count)
                    continue;
                LOGK("part %s \n", part->name);
                LOGK("    bootable %d\n", entry->bootable);
                LOGK("    start %d\n", entry->start);
                LOGK("    count %d\n", entry->count);
                LOGK("    system %#x\n", entry->system);
            }
        }
    }
}


/**
 * @brief  初始化IDE（PATA）控制器
 * @retval 无
 * @note 
 */
static void ide_ctrl_init()
{
    uint16* buf = (uint16 *)alloc_kpage(1);
    for(size_t cidx = 0; cidx < IDE_CTRL_NR; cidx++)
    {
        ide_ctrl_t * ctrl = & controllers[cidx];
        sprintf(ctrl->name, "cidx%d", cidx);
        lock_init(&ctrl->lock);
        ctrl->active = NULL;
        ctrl->waiter = NULL;

        if(cidx)
        {
            ctrl->iobase = IDE_IOBASE_SECONDARY;
        }
        else
        {
            ctrl->iobase = IDE_IOBASE_PRIMARY;
        }

        ctrl->control = inb(ctrl->iobase + IDE_CONTROL);
        for(size_t didx = 0; didx < IDE_DISK_NR; didx++)
        {
            ide_disk_t * disk = & ctrl->disk[didx];
            sprintf(disk->name, "hd%c", 'a'+cidx*2+didx);
            disk->ctrl = ctrl;
            if(didx)
            {
                disk->master = false;
                disk->selector = IDE_LBA_SLAVE;
            }
            else
            {
                disk->master = true;
                disk->selector = IDE_LBA_MASTER;
            }
            ide_identify(disk, buf);
            ide_part_init(disk, buf);
        }
    }
    free_kpage((uint32)buf, 1);
}

/**
 * @brief  IDE(PATA)初始化
 * @retval 
 * @note 包括控制器和磁盘
 */
void ide_init()
{
    LOGK("[INFO]: init\n");
    ide_ctrl_init();
    interrupt_register(IRQ_HARDDISK, ide_handler);
    interrupt_register(IRQ_HARDDISK2, ide_handler);
    interrupt_mask(IRQ_HARDDISK, true);
    interrupt_mask(IRQ_HARDDISK2, true);
    interrupt_mask(IRQ_CASCADE, true);
}