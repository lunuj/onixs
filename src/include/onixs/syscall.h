#ifndef SYSCALL_H
#define SYSCALL_H

#include <onixs/types.h>

typedef enum syscall_t{
    SYS_NR_TEST,
    SYS_NR_EXIT,
    SYS_NR_WRIET = 4,
    SYS_NR_FORK,
    SYS_NR_GETPID = 20,
    SYS_NR_BRK = 45,
    SYS_NR_GETPPID = 64,
    SYS_NR_YIELD = 158,
    SYS_NR_SLEEP = 162,
} syscall_t;

uint32 test();
void yield();
int32 write(fd_t fd, char * buf, uint32 len);
void sleep(uint32 ms);
int32 brk(void * addr);
pid_t getpid();
pid_t getppid();
void exit(int status);
pid_t fork();
#endif // SYSCALL_H