#include <onixs/fs.h>
#include <onixs/stat.h>
#include <onixs/buffer.h>
#include <onixs/device.h>
#include <onixs/assert.h>
#include <onixs/string.h>
#include <onixs/stdlib.h>
#include <onixs/debug.h>
#include <onixs/device.h>
#include <onixs/task.h>

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

void put_super(super_block_t *sb)
{
    if(!sb)
        return;
    assert(sb->count > 0);
    sb->count--;
    if(sb->count)
        return;

    sb->dev = EOF;
    iput(sb->imount);
    iput(sb->iroot);

    for (int i = 0; i < sb->desc->imap_blocks; i++)
    {
        brelse(sb->imaps[i]);
    }
    for (int i = 0; i < sb->desc->zmap_blocks; i++)
    {
        brelse(sb->zmaps[i]);
    }
    
    brelse(sb->buf);
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
        sb->count++;
        return sb;
    }
    SYS_LOG(LOG_INFO, "Reading super block of device %d\n", dev);

    sb = get_free_super();
    buffer_t *buf = bread(dev, 1);
    sb->buf = buf;
    sb->desc = (super_desc_t *)buf->data;
    sb->dev = dev;
    sb->count  = 1;

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
    
    root->iroot = iget(device->dev, 1);
    root->imount = iget(device->dev, 1);
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

int sys_mount(char *devname, char *dirname, int flags)
{
    SYS_LOG(LOG_INFO, "mount %s to %s", devname, dirname);

    inode_t *devinode = NULL;
    inode_t * dirinode = NULL;
    super_block_t *sb = NULL;

    devinode = namei(devname);
    if(!devinode)
        goto rollback;
    if(!ISBLK(devinode->desc->mode))
        goto rollback;
    
    dev_t dev = devinode->desc->zone[0];

    dirinode = namei(dirname);
    if(!dirname)
        goto rollback;
    if(!ISDIR(dirinode->desc->mode))
        goto rollback;
    if(dirinode->count!=1 || dirinode->mount)
        goto rollback;

    sb = read_super(dev);
    if(sb->imount)
        goto rollback;
    
    sb->iroot = iget(dev, 1);
    sb->imount = dirinode;
    dirinode->mount = dev;
    iput(devinode);
    return 0;

rollback:
    put_super(sb);
    iput(devinode);
    iput(dirinode);
    return EOF;
}

int sys_umount(char *target)
{
    inode_t *inode = NULL;
    super_block_t *sb = NULL;
    int ret = EOF;

    inode = namei(target);
    if(!inode)
        goto rollback;
    if(!ISBLK(inode->desc->mode) && inode->nr != 1)
        goto rollback;
    if(inode == root->imount)
        goto rollback;

    dev_t dev = inode->dev;
    if(ISBLK(inode->desc->mode))
    {
        dev = inode->desc->zone[0];
    }

    sb = get_super(dev);
    if(!sb->imount)
        goto rollback;
    
    if(!sb->imount->mount)
        SYS_LOG(LOG_INFO, "super block mount = 0");

    if(list_size(&sb->inode_list) > 1)
        goto rollback;
    
    iput(sb->iroot);
    sb->iroot = NULL;

    sb->imount->mount = 0;
    iput(sb->imount);
    sb->imount = NULL;
    ret = 0;

rollback:
    put_super(sb);
    iput(inode);
    return ret;
}

int devmkfs(dev_t dev, uint32 icount)
{
    super_block_t *sb = NULL;
    buffer_t *buf = NULL;
    int ret = EOF;

    int total_block = device_ioctl(dev, DEV_CMD_SECTOR_COUNT, NULL, 0) / BLOCK_SECS;
    assert(total_block);
    assert(icount < total_block);
    if(!icount)
    {
        icount = total_block / 3;
    }

    sb = get_free_super();
    sb->dev = dev;
    sb->count = 1;

    buf = bread(dev, 1);
    sb->buf = buf;
    buf->dirty = true;

    super_desc_t *desc = (super_desc_t *)buf->data;
    sb->desc = desc;

    desc->zones = total_block;

    int inode_blocks = div_round_up(icount * sizeof(inode_desc_t), BLOCK_SIZE);
    desc->inodes = icount;
    desc->imap_blocks = div_round_up(icount, BLOCK_BITS);

    int zcount = total_block - desc->imap_blocks - inode_blocks - 2;
    desc->zmap_blocks = div_round_up(zcount, BLOCK_SIZE);

    desc->firstdatazone = 2 + desc->imap_blocks + desc->zmap_blocks + inode_blocks;
    desc->log_zone_size = 0;
    desc->max_size = BLOCK_BITS * TOTAL_BLOCK;
    desc->magic = MINIX1_MAGIC;

    memset(sb->imaps, 0, sizeof(sb->imaps));
    memset(sb->zmaps, 0, sizeof(sb->zmaps));

    int idx = 2;
    for (int  i = 0; i < desc->imap_blocks && i < IMAP_NR; i++)
    {
        if ((sb->imaps[i] = bread(dev, idx)))
        {
            memset(sb->imaps[i]->data,0, BLOCK_SIZE);
            sb->imaps[i]->dirty = true;
            idx++;
        }
        else
        {
            break;
        }
    }
    
    for (int i = 0; i < desc->zmap_blocks && i < ZMAP_NR; i++)
    {
        if((sb->zmaps[i] = bread(dev, idx)))
        {
            memset(sb->zmaps[i]->data, 0, BLOCK_SIZE);
            sb->zmaps[i]->dirty = true;
            idx++;
        }
        else
        {
            break;
        }
    }

    balloc(dev);

    ialloc(dev);
    ialloc(dev);

    int counts[] = 
    {
        icount + 1,
        zcount
    };

    buffer_t *maps[] = {
        sb->imaps[desc->imap_blocks - 1],
        sb->imaps[desc->zmap_blocks - 1]
    };


    for (size_t i = 0; i < 2; i++)
    {
        int count = counts[i];
        buffer_t *map = maps[i];
        map->dirty = true;
        int offset = count % (BLOCK_BITS);
        int begin = (offset / 8);
        char * ptr = (char *)map->data + begin;
        memset(ptr + 1, 0xff, BLOCK_SIZE - begin - 1);
        int bits = 0x80;
        char data = 0;
        int remain = 8 - offset % 8;
        while (remain--)
        {
            data |= bits;
            bits >>= 1;
        }
        ptr[0] = data;
    }
    
    task_t *task = running_task();
    inode_t *iroot = new_inode(dev, 1);
    sb->iroot = iroot;

    iroot->desc->mode = (0777 & ~task->umask) | IFDIR;
    iroot->desc->size = sizeof(dentry_t) * 2; // 当前目录和父目录两个目录项
    iroot->desc->nlinks = 2;                  // 一个是 '.' 一个是 name

    buf = bread(dev, bmap(iroot, 0, true));
    buf->dirty = true;

    dentry_t * entry = (dentry_t *)buf->data;
    memset(entry, 0, BLOCK_SIZE);

    strcpy(entry->name, ".");
    entry->nr = iroot->nr;

    entry++;
    strcpy(entry->name, "..");
    entry->nr = iroot->nr;

    brelse(buf);
    ret = 0;
}

int sys_mkfs(char *devname, int icount)
{
    inode_t *inode = NULL;
    int ret = EOF;

    inode = namei(devname);
    if(!inode)
        goto rollback;

    if(!ISBLK(inode->desc->mode))
        goto rollback;

    dev_t dev = inode->desc->zone[0];
    assert(dev);

    ret = devmkfs(dev, icount);
rollback:
    iput(inode);
    return ret;
}