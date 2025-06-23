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

inode_t *inode_open(char *pathname, int flag, int mode)
{
    inode_t *dir = NULL;
    inode_t *inode = NULL;
    buffer_t *buf = NULL;
    dentry_t *entry = NULL;
    char *next = NULL;
    
    dir = named(pathname, &next);
    if(!dir)
        goto rollback;
    if(!*next)
        goto rollback;
    if((flag & O_TRUNC) &&((flag & O_ACCMODE) == O_RDONLY))
        flag |= O_RDWR;
    
    char *name = next;
    buf = find_entry(&dir, name, &next, &entry);
    if(buf)
    {
        inode = iget(dir->dev, entry->nr);
        goto makeup;
    }

    if(!(flag & O_CREAT))
        goto rollback;

    if(!premission(dir, P_WRITE))
        goto rollback;

    buf = add_entry(dir, name, &entry);
    entry->nr = ialloc(dir->dev);
    inode = iget(dir->dev, entry->nr);

    task_t *task = running_task();

    mode &= (0777 & ~task->umask);
    mode |= IFREG;

    inode->desc->uid = task->uid;
    inode->desc->gid = task->gid;
    inode->desc->mode = mode;
    inode->desc->mtime = time();
    inode->desc->size = 0;
    inode->desc->nlinks = 1;
    inode->buf->dirty = true;

makeup:
    if(ISDIR(inode->desc->mode) || !premission(inode, flag & O_ACCMODE))
        goto rollback;

    inode->atime = time();
    
    if(flag & O_TRUNC)
        inode_truncate(inode);
    
    brelse(buf);
    iput(dir);
    return inode;
rollback:
    return NULL;
}

// 系统调用相关
int sys_link(char *oldname, char *newname)
{
    int ret = EOF;
    buffer_t *buf = NULL;
    inode_t *dir = NULL;
    inode_t *inode = namei(oldname);
    if(!inode)
        goto rollback;

    if(ISDIR(inode->desc->mode))
        goto rollback;

    char *next = NULL;
    dir = named(newname, &next);
    if(!dir)
        goto rollback;
    
    if(!(*next))
        goto rollback;
    
    if(dir->dev != inode->dev)
        goto rollback;
    
    if(!premission(dir, P_WRITE))
        goto rollback;

    char *name = next;
    dentry_t *entry;

    buf = find_entry(&dir, name, &next, &entry);
    if(buf)
        goto rollback;

    buf = find_entry(&dir, name, &next, &entry);
    if(buf)
        goto rollback;
    
    buf = add_entry(dir, name, &entry);
    entry->nr = inode->nr;
    buf->dirty = true;

    inode->desc->nlinks++;
    inode->ctime = time();
    inode->buf->dirty = true;
    ret = 0;
rollback:
    brelse(buf);
    iput(inode);
    iput(dir);
    return ret;
}

int sys_unlink(char *filename)
{
    int ret = EOF;
    char *next = NULL;
    inode_t *inode = NULL;
    buffer_t *buf = NULL;
    inode_t *dir = named(filename, &next);
    if(!dir)
        goto rollback;
    
    if(!(*next))
        goto rollback;

    if(!premission(dir, P_WRITE));
        goto rollback;
    
    char *name = next;
    dentry_t *entry;
    buf = find_entry(&dir, name, &next, &entry);
    if(!buf)
        goto rollback;
    
    inode = iget(dir->dev, entry->nr);
    if(ISDIR(inode->desc->mode))
        goto rollback;

    task_t *task = running_task();
    if((inode->desc->mode & ISVTX) && task->uid != inode->desc->uid)
        goto rollback;

    if(!inode->desc->nlinks)
        SYS_LOG(LOG_INFO, "deleting non exists file (%04x:%d)\n", inode->dev, inode->nr);
    
    entry->nr = 0;
    buf->dirty = true;

    inode->desc->nlinks--;
    inode->buf->dirty = true;

    if (inode->desc->nlinks == 0)
    {
        inode_truncate(inode);
        ifree(inode->dev, inode->nr);
    }

    ret = 0;

rollback:
    brelse(buf);
    iput(inode);
    iput(dir);
    return ret;
}


int sys_mkdir(char *pathname, int mode)
{
    char *next = NULL;
    buffer_t *ebuf = NULL;
    inode_t *dir = named(pathname, &next);

    if(!dir)
        goto rollback;

    if(!*next)
        goto rollback;

    if(!premission(dir, P_WRITE))
        goto rollback;

    char *name = next;
    dentry_t *entry;

    ebuf = find_entry(&dir, name, &next, &entry);

    if(ebuf)
        goto rollback;

    ebuf = add_entry(dir, name, &entry);
    ebuf->dirty = true;
    entry->nr = ialloc(dir->dev);

    task_t *task = running_task();
    inode_t *inode = iget(dir->dev, entry->nr);
    inode->buf->dirty = true;

    inode->desc->gid = task->gid;
    inode->desc->uid = task->uid;
    inode->desc->mode = (mode & 0777 & ~task->umask) | IFDIR;
    inode->desc->size = sizeof(dentry_t)*2;
    inode->desc->mtime =  time();
    inode->desc->nlinks = 2;

    dir->buf->dirty = true;
    dir->desc->nlinks++;

    buffer_t *zbuf = bread(inode->dev, bmap(inode, 0, true));
    zbuf->dirty = true;

    entry = (dentry_t *)zbuf->data;

    strcpy(entry->name, ".");
    entry->nr = dir->nr;

    entry++;
    strcpy(entry->name, "..");
    entry->nr = dir->nr;
    
    iput(inode);
    iput(dir);

    brelse(ebuf);
    brelse(zbuf);
    return 0;

rollback:
    brelse(ebuf);
    iput(dir);
    return EOF;
}

static bool is_empty(inode_t *inode)
{
    assert(ISDIR(inode->desc->mode));
    int entries = inode->desc->size / sizeof(dentry_t);
    if(entries < 2 || !inode->desc->zone[0])
    {
        SYS_LOG(LOG_ERROR, "bad directory on dev %d\n", inode->dev);
        return false;
    }

    idx_t i = 0;
    idx_t block = 0;
    buffer_t *buf = NULL;
    dentry_t *entry;
    int count = 0;

    for (; i < entries; i++, entry++)
    {
        if (!buf || (uint32)entry >= (uint32)buf->data + BLOCK_SIZE)
        {
            brelse(buf);
            block = bmap(inode, i / BLOCK_DENTRIES, false);
            assert(block);

            buf = bread(inode->dev, block);
            entry = (dentry_t *)buf->data;
        }
        if(entry->nr)
            count++;
    };

    brelse(buf);

    if(count < 2)
    {
        SYS_LOG(LOG_ERROR, "bad directory on dev %d\n", inode->dev);
        return false;
    }
    return count == 2;
}

int sys_rmdir(char *pathname)
{
    char *next = NULL;
    buffer_t *ebuf = NULL;
    inode_t *dir = named(pathname, &next);
    inode_t *inode = NULL;
    int ret = EOF;

    if(!dir)
        goto rollback;

    if(!*next)
        goto rollback;
    
    if(!premission(dir, P_WRITE))
        goto rollback;

    char *name = next;
    dentry_t *entry;

    ebuf = find_entry(&dir, name, &next, &entry);
    if(!ebuf)
        goto rollback;
    
    inode = iget(dir->dev, entry->nr);
    if(!inode)
        goto rollback;
    if(inode == dir)
        goto rollback;
    if(!ISDIR(inode->desc->mode))
        goto rollback;
    task_t *task = running_task();
    if((dir->desc->mode & ISVTX) && task->uid != inode->desc->uid)
        goto rollback;
    if(dir->dev != inode->dev || inode->count > 1)
        goto rollback;
    if(!is_empty(inode))
        goto rollback;

    assert(inode->desc->nlinks == 2);

    inode_truncate(inode);
    ifree(inode->dev, inode->nr);

    inode->desc->nlinks = 0;
    inode->buf->dirty = true;
    inode->nr = 0;

    dir->desc->nlinks--;
    dir->ctime = dir->atime = dir->desc->mtime = time();
    dir->buf->dirty = true;
    assert(dir->desc->nlinks > 0);

    entry->nr = 0;
    ebuf->dirty = true;

    ret = 0;

rollback:
    iput(inode);
    iput(dir);
    brelse(ebuf);
    return ret;
}