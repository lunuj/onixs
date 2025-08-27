#include <onixs/stdio.h>
#include <onixs/syscall.h>
#include <onixs/string.h>
#include <onixs/stdlib.h>
#include <onixs/fs.h>
#include <onixs/time.h>
#include <onixs/stat.h>

#define MAX_PATH_LEN 128
#define BUFLEN 1024

static void parsemode(int mode, char *buf)
{
    memset(buf, '-', 20);
    buf[10] = '\0';
    char *ptr = buf;

    switch (mode & IFMT)
    {
    case IFREG:
        *ptr = '-';
        break;
    case IFBLK:
        *ptr = 'b';
        break;
    case IFDIR:
        *ptr = 'd';
        break;
    case IFCHR:
        *ptr = 'c';
        break;
    case IFIFO:
        *ptr = 'p';
        break;
    case IFLNK:
        *ptr = 'l';
        break;
    case IFSOCK:
        *ptr = 's';
        break;
    default:
        *ptr = '?';
        break;
    }
    ptr++;

    for (int i = 6; i >= 0; i-=3)
    {
        int fmt = (mode >> i) & 07;
        if(fmt & 0b100)
        {
            *ptr = 'r';
        }
        ptr++;
        if(fmt & 0b010)
        {
            *ptr = 'w';
        }
        ptr++;
        if(fmt & 0b01)
        {
            *ptr = 'x';
        }
        ptr++;    
    }
}

static void strftime(time_t stamp, char *buf)
{
    tm time;
    localtime(stamp, &time);
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d",
            time.tm_year + 1900,
            time.tm_mon,
            time.tm_mday,
            time.tm_hour,
            time.tm_min,
            time.tm_sec);
}

int main(int argc, char const *argv[], char const *envp[])
{
    char cwd[MAX_PATH_LEN];
    static char buf[BUFLEN];
    getcwd(cwd, MAX_PATH_LEN);
    fd_t fd = open(cwd, O_RDONLY, 0);
    if(fd == EOF)
        return 0;
    bool list = false;
    if(argc >= 2 && !strcmp(argv[1], "-l"))
        list = true;
    lseek(fd, 0, SEEK_SET);
    dentry_t entry;
    while(true)
    {
        int len = readdir(fd, &entry, 1);
        if(len == EOF)
            break;
        if(!entry.nr)
            break;
        if(!strcmp(entry.name, ".") || !strcmp(entry.name, ".."))
        {
            continue;
        }
        if(!list)
        {
            printf("%s ", entry.name);
            continue;
        }
        stat_t statbuf;
        stat(entry.name, &statbuf);
        parsemode(statbuf.mode, buf);
        printf("%s ", buf);

        strftime(statbuf.ctime, buf);
        printf("% 2d % 2d % 2d % 2d %s %s\n",
               statbuf.nlinks,
               statbuf.uid,
               statbuf.gid,
               statbuf.size,
               buf,
               entry.name);
    }
    if (!list)
        printf("\n");
    close(fd);
    return 0;
}
