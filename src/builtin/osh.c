#include <onixs/stdio.h>
#include <onixs/syscall.h>
#include <onixs/string.h>
#include <onixs/stdlib.h>
#include <onixs/assert.h>
#include <onixs/fs.h>
#include <onixs/stat.h>
#include <onixs/time.h>

#define MAX_CMD_LEN 256
#define MAX_ARG_NR 16
#define MAX_PATH_LEN 1024
#define BUFLEN 1024

static char cwd[MAX_PATH_LEN];
static char cmd[MAX_CMD_LEN];
static char *argv[MAX_ARG_NR];
static char buf[BUFLEN];

static char onixs_logo[][52] = {
    "                                ____        _     \n\t",
    "                               / __ \\___  (_)_ __ \n\t",
    "                              / /_/ / _ \\/ /\\ \\ / \n\t",
    "                              \\____/_//_/_//_\\_\\  \n\0",
};

/**
 * @brief  获取当前所在目录名
 * @param  *name 工作路径
 * @retval 当前所在目录名
 * @note 
 */
char *basename(char *name)
{
    char *ptr = strrsep(name);
    if(!ptr[1])
    {
        return ptr;
    }
    ptr++;
    return ptr;
}

/**
 * @brief  在标准输出中打印shell
 * @retval 无
 * @note 
 */
void print_prompt()
{
    getcwd(cwd, MAX_PATH_LEN);
    char *ptr = strrsep(cwd);
    if(ptr != cwd)
    {
        *ptr = 0;
    }
    char *base = basename(cwd);
    printf("[root %s]# ", base);
}

void builtin_logo()
{
    clear();
    printf((char *)onixs_logo);
}

void builtin_test(int argc, char *argv[])
{
    // test();
    uint32 status;
    int *counter = (int *)mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, EOF, 0);
    pid_t pid = fork();
    if(pid)
    {
        while(true)
        {
            (*counter)++;
            sleep((300));
        }
    }
    else
    {
        while(true)
        {
            printf("counter %d\n", *counter);
            sleep(100);
        }
    }
}

void builtin_pwd()
{
    getcwd(cwd, MAX_PATH_LEN);
    printf("%s\n",cwd);
}

void builtin_clear()
{
    clear();
}

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

void builtin_ls(int argc, char *argv[])
{
    fd_t fd = open(cwd, O_RDONLY, 0);
    if(fd == EOF)
        return;
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
}

void builtin_cd(int argc, char *argv[])
{
    if(argc == 1)
        return;
    chdir(argv[1]);
}

void builtin_cat(int argc, char *argv[])
{
    if(argc == 1)
        return;
    
    fd_t fd = open(argv[1], O_RDONLY, 0);
    if(fd == EOF)
    {
        printf("file %s not exits.\n", argv[1]);
        return;
    }

    while(true)
    {
        int len = read(fd, buf, BUFLEN);
        if(len == EOF)
            break;
        write(STDOUT_FILENO, buf, len);
    }
    close(fd);
}

void builtin_mkdir(int argc,  char *argv[])
{
    if(argc == 1)
        return;
    mkdir(argv[1], 0755);
}

void builtin_rmdir(int argc,  char *argv[])
{
    if(argc == 1)
        return;
    rmdir(argv[1]);
}

void builtin_rm(int argc, char *argv[])
{
    if(argc == 1)
        return;
    unlink(argv[1]);
}

void builtin_mount(int argc, char *argv[])
{
    if(argc < 3)
        return;
    mount(argv[1], argv[2], 0);
}

void builtin_umount(int argc, char *argv[])
{
    if(argc < 2)
        return;
    umount(argv[1]);
}

void builtin_mkfs(int argc, char *argv[])
{
    if(argc < 2)
        return;
    mkfs(argv[1], 0);
}

static void execute(int argc, char *argv[])
{
    char *line = argv[0];
    if(argc == 0)
        return;
    if(!strcmp(line, "test"))
        return builtin_test(argc, argv);
    if(!strcmp(line, "logo"))
        return builtin_logo();
    if(!strcmp(line, "pwd"))
        return builtin_pwd();
    if(!strcmp(line, "clear"))
        return builtin_clear();
    if(!strcmp(line, "exit"))
    {
        int code = 0;
        if(argc == 2)
        {
            code = atoi(argv[1]);
        }
        exit(code);
    }
    if(!strcmp(line, "ls"))
        return builtin_ls(argc, argv);
    if(!strcmp(line, "cd"))
        return builtin_cd(argc, argv);
    if(!strcmp(line, "cat"))
        return builtin_cat(argc, argv);
    if(!strcmp(line, "mkdir"))
        return builtin_mkdir(argc, argv);
    if(!strcmp(line, "rmdir"))
        return builtin_rmdir(argc, argv);
    if(!strcmp(line, "rm"))
        return builtin_rm(argc, argv);
    if(!strcmp(line, "mount"))
        return builtin_mount(argc, argv);
    if(!strcmp(line, "umount"))
        return builtin_umount(argc, argv);
    if(!strcmp(line, "mkfs"))
        return builtin_mkfs(argc, argv);
    printf("osh: command not found: %s\n", argv[0]);
}

void readline(char *buf, uint32 count)
{
    assert(buf != NULL);
    char *ptr = buf;
    uint32 idx = 0;
    char ch = 0;
    while(idx < count)
    {
        ptr = buf + idx;
        int ret = read(STDIN_FILENO, ptr, 1);
        if(ret == -1)
        {
            *ptr = 0;
            return;
        }
        switch (*ptr)
        {
        case '\n':
        case '\r':
            *ptr = 0;
            ch = '\n';
            write(STDOUT_FILENO, &ch, 1);
            return;
        case '\b':
            if(buf[0] != '\b')
            {
                idx--;
                ch = '\b';
                write(STDOUT_FILENO, &ch, 1);
            }
            break;
        case '\t':
            continue;
        default:
            write(STDOUT_FILENO, ptr, 1);
            idx++;
            break;
        }
    }
    buf[idx] = '\0';
}

static int cmd_parse(char *cmd, char *argv[], char token)
{
    assert(cmd != NULL);

    char *next = cmd;
    int argc = 0;
    while(*next && argc < MAX_ARG_NR)
    {
        while(*next == token)
        {
            next++;
        }
        if(*next == 0)
        {
            break;
        }
        argv[argc++] = next;
        while(*next && *next != token)
        {
            next++;
        }
        if(*next)
        {
            *next = 0;
            next++;
        }
    }
    argv[argc] = NULL;
    return argc;
}

int osh_main()
{
    memset(cmd, 0, sizeof(cmd));
    memset(cwd, 0, sizeof(cwd));

    builtin_logo();

    while(true)
    {
        print_prompt();
        readline(cmd, sizeof(cmd));
        if(cmd[0] == 0)
            continue;
        int argc = cmd_parse(cmd, argv, ' ');
        if(argc < 0 || argc >= MAX_ARG_NR)
            continue;
        execute(argc, argv);
    }
    return 0;
}