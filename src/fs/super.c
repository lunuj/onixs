#include <onixs/fs.h>
#include <onixs/buffer.h>
#include <onixs/device.h>
#include <onixs/assert.h>
#include <onixs/string.h>
#include <onixs/debug.h>

#define SUPER_NR 16

static super_block_t super_table[SUPER_NR];
static super_block_t *root;

/**
 * @brief  获取指定dev超级块
 * @retval 指向dev超级块的指针
 * @note 若dev无超级块，则返回NULL
 */
super_block_t *get_super(dev_t dev)
{
    for(size_t i = 0; i < SUPER_NR; i++)
    {
        super_block_t *sb = &super_table[i];
        if(sb->dev == dev)
        {
            return sb;
        }
    }
    return NULL;
}

/**
 * @brief  获取空闲超级块
 * @retval 指向空闲超级块的指针
 * @note 若无空闲超级块，则陷入panic
 */
static super_block_t *get_free_super()
{
    super_block_t *sb = get_super(EOF);
    if(sb == NULL){
        panic("[ERROR]: no more super block!!!\n");
    }
    return sb;
}

/**
 * @brief  读取超级块
 * @param  dev 要读取超级块的设备
 * @retval 指向超级块的指针
 * @note 无
 */
super_block_t *read_super(dev_t dev)
{
    super_block_t *sb = get_super(dev);
    if(sb)
    {
        return sb;
    }
    SYS_LOG(LOG_INFO, "Reading super block of device %d\n", dev);

    sb = get_free_super();
    buffer_t *buf = bread(dev, 1);
    sb->buf = buf;
    sb->desc = (super_desc_t *)buf->data;
    sb->dev = dev;

    assert(sb->desc->magic == MINIX1_MAGIC);
    
    memset(sb->imaps, 0, sizeof(sb->imaps));
    memset(sb->zmaps, 0, sizeof(sb->zmaps));

    int idx = 2;
    for(int i = 0; i < sb->desc->imap_blocks; i++)
    {
        assert(i < IMAP_NR);
        if((sb->imaps[i] = bread(dev, idx)))
            idx++;
        else
            break;
    }

    for(int i = 0; i < sb->desc->zmap_blocks; i++)
    {
        assert(i < ZEROPAD);
        if((sb->zmaps[i] = bread(dev, idx)))
            idx++;
        else
            break;
    }

    return sb;
}

/**
 * @brief  挂载根目录
 * @retval 无
 * @note 从第一块磁盘的第一个PART中挂载根目录
 */
static void mount_root()
{
    SYS_LOG(LOG_INFO, "Mount root file system...\n");
    device_t *device = device_find(DEV_IDE_PART, 0);
    assert(device);

    root = read_super(device->dev);
    
    device = device_find(DEV_IDE_PART, 0);
    assert(device);
    super_block_t *sb = read_super(device->dev);

    idx_t idx = ialloc(sb->dev);
    ifree(sb->dev, idx);

    idx = balloc(sb->dev);
    bfree(sb->dev, idx);
}

/**
 * @brief  超级块初始化
 * @retval 无
 * @note 将挂载根目录
 */
void super_init()
{
    for (size_t i = 0; i < SUPER_NR; i++)
    {
        super_block_t *sb = &super_table[i];
        sb->dev = EOF;
        sb->desc = NULL;
        sb->buf = NULL;
        sb->iroot = NULL;
        sb->imount = NULL;
        list_init(&sb->inode_list);
    }

    mount_root();
}