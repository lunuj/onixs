#include <onixs/fs.h>
#include <onixs/debug.h>
#include <onixs/bitmap.h>
#include <onixs/assert.h>
#include <onixs/string.h>
#include <onixs/buffer.h>

/**
 * @brief  分配一个文件块
 * @param  dev 硬盘设备
 * @retval 文件块索引
 * @note 
 */
idx_t balloc(dev_t dev)
{
    super_block_t *sb = get_super(dev);
    assert(sb);

    buffer_t *buf = NULL;
    idx_t bit = EOF;
    bitmap_t map;

    for (size_t i = 0; i < ZMAP_NR; i++)
    {
        buf = sb->zmaps[i];
        assert(buf);

        bitmap_make(&map, buf->data, BLOCK_SIZE, i * BLOCK_BITS + sb->desc->firstdatazone - 1);

        bit = bitmap_scan(&map, 1);
        if(bit != EOF)
        {
            assert(bit < sb->desc->zones);
            buf->dirty = true;
            break;
        }
    }
    bwrite(buf);
    return bit;
}

/**
 * @brief  释放文件块
 * @param  dev 硬盘设备
 * @param  idx 释放文件块索引
 * @retval 无
 * @note 
 */
void bfree(dev_t dev, idx_t idx)
{
    super_block_t *sb = get_super(dev);
    assert(sb != NULL);
    assert(idx < sb->desc->zones);

    buffer_t * buf;
    bitmap_t map;
    for(size_t i = 0; i < ZMAP_NR; i++)
    {
        if(idx > BLOCK_BITS * (i + 1))
        {
            continue;
        }
        buf = sb->zmaps[i];
        assert(buf);

        bitmap_make(&map, buf->data, BLOCK_SIZE, BLOCK_BITS * i + sb->desc->firstdatazone - 1);

        assert(bitmap_test(&map, idx));
        bitmap_set(&map, idx, 0);

        buf->dirty = true;
        break;
    }
    bwrite(buf);
}

/**
 * @brief  分配文件系统 inode
 * @param  dev 硬盘设备
 * @retval inode索引
 * @note 
 */
idx_t ialloc(dev_t dev)
{
    super_block_t *sb = get_super(dev);
    assert(sb);

    buffer_t *buf = NULL;
    idx_t bit = EOF;
    bitmap_t map;

    for(size_t i = 0; i < IMAP_NR; i++)
    {
        buf = sb->imaps[i];
        assert(buf);

        bitmap_make(&map, buf->data, BLOCK_SIZE, i * BLOCK_SIZE);
        bit = bitmap_scan(&map, 1);
        if(bit != EOF)
        {
            assert(bit < sb->desc->inodes);
            buf->dirty = true;
            break;
        }
    }
    bwrite(buf);
    return bit;
}

void ifree(dev_t dev, idx_t idx)
{
    super_block_t *sb = get_super(dev);
    assert(sb != NULL);
    assert(idx < sb->desc->inodes);

    buffer_t *buf;
    bitmap_t map;

    for (size_t i = 0; i < IMAP_NR; i++)
    {
        if(idx > BLOCK_BITS *(i + 1))
        {
            continue;
        }
        buf = sb->imaps[i];
        assert(buf);

        bitmap_make(&map, buf->data, BLOCK_BITS, i*BLOCK_SIZE);
        assert(bitmap_test(&map, idx));
        bitmap_set(&map, idx, 0);
        buf->dirty = true;
        break;
    }
    bwrite(buf);
}