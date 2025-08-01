#include <onixs/gate.h>
#include <onixs/syscall.h>
#include <onixs/task.h>
#include <onixs/console.h>
#include <onixs/memorry.h>
#include <onixs/clock.h>
#include <onixs/device.h>
#include <onixs/buffer.h>
#include <onixs/fs.h>
#include <onixs/stat.h>

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
    SYS_LOG(LOG_INFO, "syscall test...\n");
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
    syscall_table[SYS_NR_CHDIR] = sys_chdir;
    syscall_table[SYS_NR_TIME] = sys_time;
    syscall_table[SYS_NR_MKNOD] = sys_mknod;
    syscall_table[SYS_NR_STAT] = sys_stat;
    syscall_table[SYS_NR_LSEEK] = sys_lseek;
    syscall_table[SYS_NR_GETPID] = sys_getpid;
    syscall_table[SYS_NR_MOUNT] = sys_mount;
    syscall_table[SYS_NR_UMOUNT] = sys_umount;
    syscall_table[SYS_NR_MKDIR] = sys_mkdir;
    syscall_table[SYS_NR_RMDIR] = sys_rmdir;
    syscall_table[SYS_NR_BRK] = sys_brk;
    syscall_table[SYS_NR_UMASK] = sys_umask;
    syscall_table[SYS_NR_CHROOT] = sys_chroot;
    syscall_table[SYS_NR_GETPPID] = sys_getppid;
    syscall_table[SYS_NR_MMAP] = sys_mmap;
    syscall_table[SYS_NR_MUNMAP] = sys_munmap;
    syscall_table[SYS_NR_READDIR] = sys_readdir;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;
    syscall_table[SYS_NR_GETCWD] = sys_getcwd;
    syscall_table[SYS_NR_CLEAR] = console_clear;
    syscall_table[SYS_NR_MKFS] = sys_mkfs;
}