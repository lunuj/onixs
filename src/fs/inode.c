#include <onixs/fs.h>
#include <onixs/syscall.h>
#include <onixs/assert.h>
#include <onixs/debug.h>
#include <onixs/buffer.h>
#include <onixs/arena.h>
#include <onixs/string.h>
#include <onixs/stdlib.h>
#include <onixs/stat.h>

#define INODE_NR 64

static inode_t inode_table[INODE_NR];

/**
 * @brief  获取空闲inode
 * @retval 指向inode指针
 * @note 
 */
static inode_t *get_free_inode()
{
    for(size_t i = 0; i < INODE_NR; i++)
    {
        inode_t *inode = &inode_table[i];
        if(inode->dev == EOF)
        {
            return inode;
        }
    }
    panic("no more inode!!!");
}

/**
 * @brief  释放inode
 * @param  *inode 指向inode的指针
 * @retval 无
 * @note 
 */
static void put_free_inode(inode_t *inode)
{
    assert(inode != inode_table);
    assert(inode->count == 0);
    inode->dev = EOF;
}

/**
 * @brief  获取根节点inode
 * @retval 指向根节点inode指针
 * @note 
 */
inode_t *get_root_inode()
{
    return inode_table;
}

/**
 * @brief  查找dev设备对应nr编号的inode
 * @param  dev 存储设备
 * @param  nr inode编号
 * @retval 
 * @note 
 */
static inode_t *find_inode(dev_t dev, idx_t nr)
{
    super_block_t *sb = get_super(dev);
    assert(sb);
    list_t *list = &sb->inode_list;
    for(list_node_t *node = list->head.next; node != &list->tail; node = node->next)
    {
        inode_t *inode = element_entry(inode_t, node, node);
        if(inode->nr == nr)
        {
            return inode;
        }
    }
    return NULL;
}

/**
 * @brief  获取inode对应块号
 * @param  *sb 超级块指针
 * @param  nr inode编号，从1开始
 * @retval inode所对应的块号
 * @note 
 */
static inline idx_t inode_block(super_block_t *sb, idx_t nr)
{
    return 2 + sb->desc->imap_blocks + sb->desc->zmap_blocks + (nr -1)/BLOCK_INODES;
}

inode_t *iget(dev_t dev, idx_t nr)
{
    inode_t *inode = find_inode(dev, nr);
    if(inode)
    {
        inode->count++;
        inode->atime = time();

        return inode;
    }

    super_block_t *sb = get_super(dev);
    assert(sb);
    assert(nr <= sb->desc->inodes);

    inode = get_free_inode();
    inode->dev = dev;
    inode->nr = nr;
    inode->count = 1;

    list_push(&sb->inode_list, &inode->node);
    idx_t block = inode_block(sb, nr);
    buffer_t *buf = bread(inode->dev, block);

    inode->buf = buf;

    inode->desc = &((inode_desc_t *)buf->data)[(inode->nr -1) % BLOCK_INODES];

    inode->ctime = inode->desc->mtime;
    inode->atime = time();

    return inode;
}

/**
 * @brief  释放对应inode
 * @param  *inode 指向inode指针
 * @retval 无
 * @note 
 */
void iput(inode_t *inode)
{
    if(!inode)
        return;

    if(inode->buf->dirty)
    {
        bwrite(inode->buf);
    }

    inode->count--;
    if(inode->count)
    {
        return;
    }

    brelse(inode->buf);
    list_remove(&inode->node);
    put_free_inode(inode);
}

void inode_init()
{
    for (size_t i = 0; i < INODE_NR; i++)
    {
        inode_t *inode = &inode_table[i];
        inode->dev = EOF;
    }
}

int inode_read(inode_t *inode, char *buf, uint32 len, off_t offset)
{
    assert(ISFILE(inode->desc->mode) || ISDIR(inode->desc->mode));

    if(offset > inode->desc->size)
        return EOF;

    uint32 begin = offset;

    uint32 left = MIN(len, inode->desc->size - offset);
    while (left)
    {
        idx_t nr = bmap(inode, offset/BLOCK_SIZE, false);
        assert(nr);

        buffer_t *bf = bread(inode->dev, nr);
        
        uint32 start = offset%BLOCK_SIZE;
        uint32 chars = MIN(BLOCK_SIZE - start, left);

        char *ptr = bf->data + start;
        memcpy(buf, ptr, chars);

        offset += chars;
        left -= chars;

        buf += chars;
        brelse(bf);
    }
    inode->atime = time();
    return offset - begin;
}

int inode_write(inode_t *inode, char *buf, uint32 len, off_t offset)
{
    assert(ISFILE(inode->desc->mode));

    uint32 begin = offset;
    uint32 left = len;

    while(left)
    {
        idx_t nr = bmap(inode, offset/BLOCK_SIZE, true);
        assert(nr);

        buffer_t *bf = bread(inode->dev, nr);
        bf->dirty = true;

        uint32 start = offset%BLOCK_SIZE;
        uint32 chars = MIN(BLOCK_SIZE - start, left);

        char *ptr = bf->data + start;
        memcpy(ptr, buf, chars);

        offset += chars;
        left -= chars;
        buf += chars;
        // 如果偏移量大于文件大小，则更新
        if (offset > inode->desc->size)
        {
            inode->desc->size = offset;
            inode->buf->dirty = true;
        }

        brelse(bf);
    }
    inode->desc->mtime = inode->atime = time();

    bwrite(inode->buf);
    return offset - begin;
}

static void inode_bfree(inode_t *inode, uint16 *arrary, int index, int level)
{
    if(!arrary[index])
    {
        return;
    }

    if(!level)
    {
        bfree(inode->dev, arrary[index]);
        return;
    }

    buffer_t *buf = bread(inode->dev, arrary[index]);
    for(size_t i = 0; i < BLOCK_INDEXES; i++)
    {
        inode_bfree(inode, (uint16 *)buf->data, i, level - 1);
    }
    brelse(buf);
    bfree(inode->dev, arrary[index]);
}

void inode_truncate(inode_t *inode)
{
    if(!ISFILE(inode->desc->mode) && !ISDIR(inode->desc->mode))
    {
        return;
    }

    for (size_t i = 0; i < DIRECT_BLOCK; i++)
    {
        inode_bfree(inode, inode->desc->zone, i, 0);
        inode->desc->zone[i] = 0;
    }

    inode_bfree(inode, inode->desc->zone, DIRECT_BLOCK, 1);

    inode_bfree(inode, inode->desc->zone, DIRECT_BLOCK + 1, 2);
    inode->desc->zone[DIRECT_BLOCK] = 0;
    inode->desc->zone[DIRECT_BLOCK + 1] = 0;
    inode->desc->size = 0;
    inode->buf->dirty = true;
    inode->desc->mtime = time();
    bwrite(inode->buf);
}