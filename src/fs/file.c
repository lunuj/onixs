#include <onixs/fs.h>
#include <onixs/assert.h>
#include <onixs/task.h>
#include <onixs/device.h>
#include <onixs/stat.h>

#define FILE_NR 128

file_t file_table[FILE_NR];

file_t *get_file()
{
    for(size_t i = 3; i < FILE_NR; i++)
    {
        file_t *file = &file_table[i];
        if(!file->count)
        {
            file->count++;
            return file;
        }
    }
    panic("Execeed max open files!!!");
}

void put_file(file_t *file)
{
    assert(file->count > 0);
    file->count--;
    if(!file->count)
        iput(file->inode);
}

void file_init()
{
    for (size_t i = 3; i < FILE_NR; i++)
    {
        file_t *file = &file_table[i];
        file->mode = 0;
        file->count = 0;
        file->flags = 0;
        file->offset = 0;
        file->inode = NULL;
    }
}

// 系统调用相关
int sys_read(fd_t fd, char *buf, uint32 len)
{
    task_t *task = running_task();
    file_t *file = task->files[fd];
    assert(file);
    int count = 0;

    if ((file->flags & O_ACCMODE)==O_WRONLY)
        return EOF;

    inode_t *inode = file->inode;
    if(ISCHR(inode->desc->mode))
    {
        assert(inode->desc->zone[0]);
        count = device_read(inode->desc->zone[0], buf, len, 0, 0);
        return count;
    }
    else if(ISBLK(inode->desc->mode))
    {
        assert(inode->desc->zone[0]);
        assert(file->offset % BLOCK_SIZE == 0);
        assert(len % BLOCK_BITS == 0);
        count = device_read(inode->desc->zone[0], buf, len/BLOCK_SIZE,
                            file->offset/BLOCK_SIZE, 0);
        return count;
    }else
    {
        count = inode_read(inode, buf, len, file->offset);
    }
    if(count != EOF)
    {
        file->offset += count;
    }
    return count;
}
int sys_write(fd_t fd, char *buf, uint32 len)
{
    task_t *task = running_task();
    file_t *file = task->files[fd];
    
    assert(file);

    if((file->flags & O_ACCMODE) == O_RDONLY)
        return EOF;
    
    int count = 0;
    inode_t *inode = file->inode;
    assert(inode);

    if(ISCHR(inode->desc->mode))
    {
        assert(inode->desc->zone[0]);
        count = device_write(inode->desc->zone[0], buf, len, 0, 0);
        return count;
    }
    else if(ISBLK(inode->desc->mode))
    {
        assert(inode->desc->zone[0]);
        assert(file->offset % BLOCK_SIZE == 0);
        assert(len % BLOCK_SIZE == 0);
        device_write(inode->desc->zone[0], buf, len/BLOCK_SIZE, file->offset/BLOCK_SIZE, 0);
        return count;
    }
    else
    {
        count = inode_write(inode, buf, len, file->offset);

    }
    if(count != EOF)
    {
        file->offset += count;
    }
    return count;
}


fd_t sys_open(char *filename, int flags, int mode)
{
    inode_t *inode = inode_open(filename, flags, mode);
    if(!inode)
        return EOF;
    task_t *task = running_task();
    fd_t fd = task_get_fd(task);
    file_t *file = get_file();
    assert(task->files[fd] == NULL);
    task->files[fd] = file;

    file->inode = inode;
    file->flags = flags;
    file->count = 1;
    file->mode = inode->desc->mode;
    file->offset = 0;
    if(flags & O_APPEND)
    {
        file->offset = file->inode->desc->size;
    }
    return fd;
}

void sys_close(fd_t fd)
{
    assert(fd < TASK_FILE_NR);
    task_t *task = running_task();
    file_t *file = task->files[fd];
    if(!file)
        return;
    assert(file->inode);
    put_file(file);
    task_put_fd(task, fd);
}

fd_t sys_creat(char *filename, int mode)
{
    return sys_open(filename, O_CREAT | O_TRUNC, mode);
}

int sys_lseek(fd_t fd, off_t offset, int whence)
{
    assert(fd < TASK_FILE_NR);

    task_t *task = running_task();
    file_t *file = task->files[fd];

    assert(file);
    assert(file->inode);

    switch (whence)
    {
    case SEEK_SET:
        assert(offset >= 0);
        file->offset = offset;
        break;
    case SEEK_CUR:
        assert(file->offset + offset >= 0);
        file->offset += offset;
        break;
    case SEEK_END:
        assert(file->inode->desc->size + offset >= 0);
        file->offset = file->inode->desc->size + offset;
        break;
    default:
        panic("whence not defined!");
        break;
    }
    return file->offset;
}

int sys_readdir(fd_t fd, dirent_t *dir, uint32 count)
{
    return sys_read(fd, (char *)dir, sizeof(dirent_t));
}