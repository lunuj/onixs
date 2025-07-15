#ifndef SYSCALL_H
#define SYSCALL_H

#include <onixs/types.h>
#include <onixs/stat.h>

typedef enum syscall_t{
    SYS_NR_TEST,
    SYS_NR_EXIT = 1,
    SYS_NR_FORK = 2,
    SYS_NR_READ = 3,
    SYS_NR_WRITE = 4,
    SYS_NR_OPEN = 5,
    SYS_NR_CLOSE = 6,
    SYS_NR_WAITPID = 7,
    SYS_NR_CREAT = 8,
    SYS_NR_LINK = 9,
    SYS_NR_UNLINK = 10,
    SYS_NR_CHDIR = 12,
    SYS_NR_TIME = 13,
    SYS_NR_MKNOD = 14,
    SYS_NR_STAT = 18,
    SYS_NR_LSEEK = 19,
    SYS_NR_GETPID = 20,
    SYS_NR_FSTAT = 28,
    SYS_NR_MKDIR = 39,
    SYS_NR_RMDIR = 40,
    SYS_NR_BRK = 45,
    SYS_NR_UMASK = 60,
    SYS_NR_CHROOT = 61,
    SYS_NR_GETPPID = 64,
    SYS_NR_READDIR = 89,
    SYS_NR_YIELD = 158,
    SYS_NR_SLEEP = 162,
    SYS_NR_GETCWD = 183,
    SYS_NR_CLEAR = 200,
} syscall_t;

uint32 test();
void exit(int status);
pid_t fork();

int read(fd_t fd, char *buf, uint32 len);
int32 write(fd_t fd, char * buf, uint32 len);

fd_t open(char *filename, int flahs, int mode);
void close(fd_t fd);

pid_t waitpid(pid_t pid, int * status);

fd_t creat(char *filename, int mode);

int link(char *oldname, char *newname);
int unlink(char *filename);
int chdir(char *pathname);

time_t time();
int mknod(char *filename, int mode, int dev);

int stat(char *filename, stat_t *statbuf);
int lseek(fd_t fd, off_t offset, int whence);

pid_t getpid();
pid_t getppid();

int fstat(fd_t fd, stat_t *statbuf);

int mkdir(char *pathname, int mode);
int rmdir(char *pathname);

int32 brk(void * addr);

mode_t umask(mode_t mask);
int chroot(char *pathname);

void yield();
void sleep(uint32 ms);

char *getcwd(char *buf, size_t size);

int readdir(fd_t fd, void *dir, uint32 count);

void clear();
#endif // SYSCALL_H