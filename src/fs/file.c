#include <onixs/fs.h>
#include <onixs/assert.h>
#include <onixs/task.h>

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
    for (size_t i = 0; i < FILE_NR; i++)
    {
        file_t *file = &file_table[i];
        file->mode = 0;
        file->count = 0;
        file->flags = 0;
        file->offset = 0;
        file->inode = NULL;
    }
}