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

uint32 test()
{
    return _syscall0(SYS_NR_TEST);
}

void yield()
{
    _syscall0(SYS_NR_YIELD);
}

void sleep(uint32 ms)
{
    _syscall1(SYS_NR_SLEEP, ms);
}

int32 write(fd_t fd, char * buf, uint32 len)
{
    return _syscall3(SYS_NR_WRITE, fd, (uint32)buf, len);
}

int32 brk(void * addr)
{
    return _syscall1(SYS_NR_BRK, (uint32)addr);
}

pid_t getpid(){
    return _syscall0(SYS_NR_GETPID);
}

pid_t getppid(){
    return _syscall0(SYS_NR_GETPPID);
}

pid_t fork(){
    return _syscall0(SYS_NR_FORK);
}

void exit(int status){
    _syscall1(SYS_NR_EXIT, status);
}

fd_t open(char *filename, int flags, int mode)
{
    return _syscall3(SYS_NR_OPEN, (uint32)filename, (uint32)flags, (uint32)mode);
}

void close(fd_t fd)
{
    _syscall1(SYS_NR_CLOSE, (uint32)fd);
}

fd_t creat(char *filename, int mode)
{
    return _syscall2(SYS_NR_CREAT, (uint32)filename, (uint32)mode);
}

pid_t waitpid(pid_t pid, int * status)
{
    return _syscall2(SYS_NR_WAITPID, pid, (uint32)status);
}

int mkdir(char *pathname, int mode)
{
    return _syscall2(SYS_NR_MKDIR, (uint32)pathname, (uint32)mode);
}

int rmdir(char *pathname)
{
    return _syscall1(SYS_NR_RMDIR, (uint32)pathname);
}

int link(char *oldname, char *newname)
{
    return _syscall2(SYS_NR_LINK, (uint32)oldname, (uint32)newname);
}

int unlink(char *filename)
{
    return _syscall1(SYS_NR_LINK, (uint32)filename);
}

time_t time()
{
    return _syscall0(SYS_NR_TIME);
}

mode_t umask(mode_t mask)
{
    return _syscall1(SYS_NR_UMASK, (uint32)mask);
}