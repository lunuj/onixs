#ifndef DEVICE_H
#define DEVICE_H

#include <onixs/types.h>
#include <onixs/list.h>

#define DEVICE_NAME_LEN 16

#define DIRECT_UP 0   // 上楼
#define DIRECT_DOWN 1 // 下楼

enum device_type_t
{
    DEV_NULL,
    DEV_CHAR,
    DEV_BLOCK
};

enum device_subtype_t
{
    DEV_CONSOLE = 1,
    DEV_KEYBOARD,
    DEV_SERIAL,         // 串口
    DEV_IDE_DISK,       // IDE 磁盘
    DEV_IDE_PART,       // IDE 磁盘分区
    DEV_RAMDISK,        // 虚拟磁盘
};

// 设备控制命令
enum device_cmd_t
{
    DEV_CMD_SECTOR_START = 1, // 获得设备扇区开始位置 lba
    DEV_CMD_SECTOR_COUNT,     // 获得设备扇区数量
};

#define REQ_READ 0  // 块设备读
#define REQ_WRITE 1 // 块设备写

// 块设备请求
typedef struct request_t
{
    dev_t dev;              // 设备号
    uint32 type;            // 请求类型
    uint32 idx;             // 扇区位置
    uint32 count;           // 扇区数量
    int flags;              // 特殊标志
    uint8 *buf;             // 缓冲区
    struct task_t *task;    // 请求进程
    list_node_t node;       // 列表节点
} request_t;

typedef struct device_t
{
    char name[DEVICE_NAME_LEN];
    int type;
    int subtype;
    dev_t dev;
    dev_t parent;
    void *ptr;
    list_t request_list; // 块设备请求链表
    bool direct;         // 磁盘寻道方向
    int (*ioctl)(void *dev, int cmd, void *args, int flags);
    int (*read)(void *dev, void *buf, size_t count ,idx_t ide, int flags);
    int (*write)(void *dev, void *buf, size_t count ,idx_t ide, int flags);
} device_t;

dev_t device_install(
    int type, int subtype,
    void *ptr, char *name, dev_t parent,
    void *ioctl, void *read, void *write);

device_t *device_find(int type, idx_t idx);
device_t *device_get(dev_t dev);

int device_ioctl(dev_t dev, int cmd, void *args, int flags);
int device_read(dev_t dev, void *buf, size_t count ,idx_t ide, int flags);
int device_write(dev_t dev, void *buf, size_t count ,idx_t ide, int flags);
// 块设备请求
void device_request(dev_t dev, void *buf, uint8 count, idx_t idx, int flags, uint32 type);
void serial_init();
#endif // DEVICE_H