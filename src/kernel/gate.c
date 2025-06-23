#include <onixs/gate.h>
#include <onixs/syscall.h>
#include <onixs/task.h>
#include <onixs/console.h>
#include <onixs/memorry.h>
#include <onixs/clock.h>
#include <onixs/device.h>
#include <onixs/buffer.h>
#include <onixs/fs.h>

handler_t syscall_table[SYSCALL_SIZE];

void syscall_check(uint32 nr){
    if(nr >= SYSCALL_SIZE){
        panic("[ERROR]: syscall nr error");
    }
}

static void sys_default()
{
    panic("[ERROR]: syscall not implemented");
}

static uint32 sys_test(){
    // LOGK("syscall test...\n");
    inode_t *inode = inode_open("/hello.txt", O_RDWR | O_CREAT, 0755);
    assert(inode);

    char *buf = (char *)alloc_kpage(1);
    int i = inode_read(inode, buf, 1024, 0);

    memset(buf, 'A', 4096);
    inode_write(inode, buf, 4096, 0);

    iput(inode);

    char ch;
    device_t *device = device_find(DEV_KEYBOARD, 0);
    assert(device);
    device_read(device->dev, &ch, 1, 0, 0);

    device = device_find(DEV_CONSOLE, 0);
    assert(device);
    device_write(device->dev, &ch, 1, 0, 0);
    
    return 255;
}

extern uint32 startup_time;
time_t sys_time()
{
    return startup_time + (jiffies * JIFFY) /1000;
}

void syscall_init(){
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = sys_default;
    }
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_EXIT] = task_exit;
    syscall_table[SYS_NR_FORK] = task_fork;
    syscall_table[SYS_NR_READ] = sys_read;
    syscall_table[SYS_NR_WRITE] = sys_write;
    syscall_table[SYS_NR_OPEN] = sys_open;
    syscall_table[SYS_NR_CLOSE] = sys_close;
    syscall_table[SYS_NR_WAITPID] = task_waitpid;
    syscall_table[SYS_NR_CREAT] = sys_creat;
    syscall_table[SYS_NR_LINK] = sys_link;
    syscall_table[SYS_NR_UNLINK] = sys_unlink;
    syscall_table[SYS_NR_TIME] = sys_time;
    syscall_table[SYS_NR_GETPID] = sys_getpid;
    syscall_table[SYS_NR_GETPPID] = sys_getppid;
    syscall_table[SYS_NR_MKDIR] = sys_mkdir;
    syscall_table[SYS_NR_RMDIR] = sys_rmdir;
    syscall_table[SYS_NR_BRK] = sys_brk;
    syscall_table[SYS_NR_UMASK] = sys_umask;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;
}