#include <onixs/syscall.h>

static _inline uint32 _syscall0(uint32 nr)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr));
    return ret;
}

static _inline uint32 _syscall1(uint32 nr, uint32 arg)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr),"b"(arg));
    return ret;
}

static _inline uint32 _syscall2(uint32 nr, uint32 arg1, uint32 arg2)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr),"b"(arg1),"c"(arg2));
    return ret;
}

static _inline uint32 _syscall3(uint32 nr, uint32 arg1, uint32 arg2, uint32 arg3)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr),"b"(arg1),"c"(arg2),"d"(arg3));
    return ret;
}

static _inline uint32 _syscall4(uint32 nr, uint32 arg1, uint32 arg2,
                                uint32 arg3, uint32 arg4)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr),"b"(arg1),"c"(arg2),"d"(arg3), "S"(arg4));
    return ret;
}

static _inline uint32 _syscall5(uint32 nr, uint32 arg1, uint32 arg2,
                                uint32 arg3, uint32 arg4, uint32 arg5)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr),"b"(arg1),"c"(arg2),"d"(arg3), "S"(arg4), "D"(arg5));
    return ret;
}

static _inline uint32 _syscall6(uint32 nr, uint32 arg1, uint32 arg2,
                                uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6)
{
    uint32 ret;
    asm volatile(
        "pushl %%ebp\n"
        "movl %7, %%ebp\n"
        "int $0x80\n"
        "popl %%ebp" 
        : "=a"(ret)
        : "a"(nr),"b"(arg1),"c"(arg2),"d"(arg3), "S"(arg4), "D"(arg5), "m"(arg6));
    return ret;
}


uint32 test()
{
    return _syscall0(SYS_NR_TEST);
}

void exit(int status)
{
    _syscall1(SYS_NR_EXIT, status);
}
pid_t fork()
{
    return _syscall0(SYS_NR_FORK);
}

int read(fd_t fd, char *buf, uint32 len)
{
    return _syscall3(SYS_NR_READ, (uint32)fd, (uint32)buf, (uint32)len);
}
int write(fd_t fd, char * buf, uint32 len)
{
    return _syscall3(SYS_NR_WRITE, fd, (uint32)buf, len);
}

fd_t open(char *filename, int flags, int mode)
{
    return _syscall3(SYS_NR_OPEN, (uint32)filename, (uint32)flags, (uint32)mode);
}
void close(fd_t fd)
{
    _syscall1(SYS_NR_CLOSE, (uint32)fd);
}

pid_t waitpid(pid_t pid, int * status)
{
    return _syscall2(SYS_NR_WAITPID, pid, (uint32)status);
}
fd_t creat(char *filename, int mode)
{
    return _syscall2(SYS_NR_CREAT, (uint32)filename, (uint32)mode);
}

int link(char *oldname, char *newname)
{
    return _syscall2(SYS_NR_LINK, (uint32)oldname, (uint32)newname);
}
int unlink(char *filename)
{
    return _syscall1(SYS_NR_UNLINK, (uint32)filename);
}
int chdir(char *pathname)
{
    return _syscall1(SYS_NR_CHDIR, (uint32)pathname);
}

time_t time()
{
    return _syscall0(SYS_NR_TIME);
}
int mknod(char *filename, int mode, int dev)
{
    return _syscall3(SYS_NR_MKNOD, (uint32)filename, (uint32)mode, (uint32)dev);
}

int stat(char *filename, stat_t *statbuf)
{
    return _syscall2(SYS_NR_STAT, (uint32)filename, (uint32)statbuf);
}
int lseek(fd_t fd, off_t offset, int whence)
{
    return _syscall3(SYS_NR_LSEEK, (uint32)fd, (uint32)offset, (uint32)whence);
}

pid_t getpid(){
    return _syscall0(SYS_NR_GETPID);
}
int mount(char *devname, char *dirname, int flags)
{
    return _syscall3(SYS_NR_MOUNT, (uint32)devname, (uint32)dirname, (uint32)flags);
}
int umount(char *target)
{
    return _syscall1(SYS_NR_UMOUNT, (uint32)target);
}
int fstat(fd_t fd, stat_t *statbuf)
{
    return _syscall2(SYS_NR_FSTAT, (uint32)fd, (uint32)statbuf);
}

int mkdir(char *pathname, int mode)
{
    return _syscall2(SYS_NR_MKDIR, (uint32)pathname, (uint32)mode);
}
int rmdir(char *pathname)
{
    return _syscall1(SYS_NR_RMDIR, (uint32)pathname);
}

int32 brk(void * addr)
{
    return _syscall1(SYS_NR_BRK, (uint32)addr);
}

mode_t umask(mode_t mask)
{
    return _syscall1(SYS_NR_UMASK, (uint32)mask);
}
int chroot(char *pathname)
{
    return _syscall1(SYS_NR_CHROOT, (uint32)pathname);
}

pid_t getppid(){
    return _syscall0(SYS_NR_GETPPID);
}

int readdir(fd_t fd, void *dir, uint32 count)
{
    return _syscall3(SYS_NR_READDIR, fd, (uint32)dir, (uint32)count);
}
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return (void *)_syscall6(SYS_NR_MMAP, (uint32)addr, (uint32)length,
                                (uint32)prot, (uint32)flags, (uint32)fd, (uint32)offset);
}
int munmap(void *addr, size_t length)
{
    return _syscall2(SYS_NR_MUNMAP, (uint32)addr, (uint32)length);
}

void yield()
{
    _syscall0(SYS_NR_YIELD);
}
void sleep(uint32 ms)
{
    _syscall1(SYS_NR_SLEEP, ms);
}

char *getcwd(char *buf, size_t size)
{
    return (char *)_syscall2(SYS_NR_GETCWD, (uint32)buf, (uint32)size);
}

void clear()
{
    _syscall0(SYS_NR_CLEAR);
}

int mkfs(char *devname, int icount)
{
    _syscall2(SYS_NR_MKFS, (uint32)devname, (uint32)icount);
}