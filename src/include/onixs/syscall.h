#ifndef SYSCALL_H
#define SYSCALL_H

#include <onixs/types.h>

typedef enum syscall_t{
    SYS_NR_TEST,
    SYS_NR_WRIET,
    SYS_NR_SLEEP,
    SYS_NR_YIELD,
} syscall_t;

uint32 test();
void yield();
int32 write(fd_t fd, char * buf, uint32 len);
void sleep(uint32 ms);

#endif // SYSCALL_H