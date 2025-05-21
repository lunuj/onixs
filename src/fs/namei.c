#include <onixs/string.h>
#include <onixs/assert.h>
#include <onixs/debug.h>
#include <onixs/assert.h>
#include <onixs/fs.h>
#include <onixs/buffer.h>
#include <onixs/syscall.h>
#include <onixs/stat.h>
#include <onixs/task.h>

#define P_EXEC IXOTH
#define P_READ IROTH
#define P_WRITE IWOTH

static bool match_name(const char *name, const char *entry_name, char **next)
{
    char *lhs = (char *)name;
    char *rhs = (char *)entry_name;
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    if(*rhs)
        return false;
    if(*lhs && !IS_SEPARATOR(*lhs))
        return false;
    if(IS_SEPARATOR(*lhs))
        lhs++;
    *next = lhs;
    return true;
    
}

static buffer_t *find_entry(inode_t **dir, const char *name, char **next, dentry_t **result)
{
    assert(ISDIR((*dir)->desc->mode));
    uint32 entries = (*dir)->desc->size / sizeof(dentry_t);

    idx_t i = 0;
    idx_t block = 0;
    buffer_t *buf = NULL;
    dentry_t *entry = NULL;
    idx_t nr = EOF;

    for ( ; i < entries; i++, entry++)
    {
        if(!buf || (uint32)entry >= (uint32)buf->data + BLOCK_SIZE)
        {
            brelse(buf);
            block = bmap((*dir), i / BLOCK_DENTRIES, false);
            assert(block);

            buf = bread((*dir)->dev, block);
            entry = (dentry_t *)buf->data;
        }
        if(match_name(name, entry->name, next))
        {
            *result = entry;
            return buf;
        }
    }
    brelse(buf);
    return NULL;
}

static buffer_t *add_entry(inode_t *dir, const char *name, dentry_t **result)
{
    char *next = NULL;

    buffer_t *buf = find_entry(&dir, name, &next, result);
    if(buf)
    {
        return buf;
    }

    for (size_t i = 0; i < NAME_LEN && name[i]; i++)
    {
        assert(!IS_SEPARATOR(name[i]));
    }

    idx_t i = 0;
    idx_t block = 0;
    dentry_t *entry;

    for ( ; true; i++, entry++)
    {
        if(!buf || (uint32)entry >= (uint32)buf->data + BLOCK_SIZE)
        {
            brelse(buf);
            block = bmap(dir, i / BLOCK_DENTRIES, true);
            assert(block);

            buf = bread(dir->dev, block);
            entry = (dentry_t *)buf->data;
        }
        if(i * sizeof(dentry_t) >= dir->desc->size)
        {
            entry->nr = 0;
            dir->desc->size = (i + 1) * sizeof(dentry_t);
            dir->buf->dirty = true;
        }
        if(entry->nr)
            continue;
        strncpy(entry->name, name, NAME_LEN);
        buf->dirty = true;
        dir->desc->mtime = time();
        dir->buf->dirty = true;
        *result = entry;
        return buf;
    }   
}

static bool premission(inode_t *inode, uint16 mask)
{
    uint16 mode = inode->desc->mode;
    if(!inode->desc->nlinks)
        return false;

    task_t *task = running_task();
    if(task->uid == KERNEL_USER)
        return true;

    if(task->uid == inode->desc->uid)
    {
        mode >>= 6;
    }else if(task->gid == inode->desc->gid)
        mode >>= 3;
    
    if((mode & mask & 0b111) == mask)
        return true;
    return false;
}

inode_t *named(char *pathname, char **next)
{
    inode_t *inode = NULL;
    task_t *task = running_task();
    char *left = pathname;
    if(IS_SEPARATOR(left[0]))
    {
        inode = task->iroot;
        left++;
    }else if(left[0])
    {
        inode = task->ipwd;
    }else{
        return NULL;
    }

    inode->count++;
    
    // left为空则为根目录，无文件
    *next = left;
    if(!*left)
    {
        return inode;
    }

    // 寻找最右侧文件名索引
    char *right = strrsep(left);
    if(!right || right < left)
    {
        return inode;
    }
    right++;    

    dentry_t *entry = NULL;
    buffer_t *buf = NULL;
    while(true)
    {
        brelse(buf);
        buf = find_entry(&inode, left, next, &entry);
        if(!buf)
            goto failure;
        
        dev_t dev = inode->dev;

        iput(inode);
        inode = iget(dev, entry->nr);
        if(!ISDIR(inode->desc->mode) || !premission(inode, P_EXEC))
            goto failure;
        if(right == *next)
            goto success;
        left = *next;
    }

brelse(buf);
success:
    return inode;
failure:
    iput(inode);
    return NULL;
}

inode_t *namei(char *pathname)
{
    char *next = NULL;
    inode_t *dir = named(pathname, &next);
    if(!dir)
        return NULL;
    if(!(*next))
        return dir;
    
    char *name = next;
    dentry_t *entry = NULL;
    buffer_t *buf = find_entry(&dir, name, &next, &entry);
    if(!buf)
    {
        iput(dir);
        return NULL;
    }

    inode_t *inode = iget(dir->dev, entry->nr);

    iput(dir);
    brelse(buf);
    return inode;
}

// TEST
#include <onixs/memorry.h>

void dir_test()
{
    inode_t *inode = namei("/d1/d2/d3/../../../hello.txt");
    char *buf = (char *)alloc_kpage(1);
    int i = inode_read(inode, buf, 1024, 0);

    SYS_LOG(LOG_INFO, "content: %s\n", buf);

    memset(buf, 'A', MEMORY_PAGE_SIZE);
    inode_write(inode, buf, MEMORY_PAGE_SIZE, 0);

    memset(buf, 'B', MEMORY_PAGE_SIZE);
    inode_write(inode, buf, MEMORY_PAGE_SIZE, MEMORY_PAGE_SIZE);
    SYS_LOG(LOG_INFO, "get inode %d\n", inode->nr);
    iput(inode);
}