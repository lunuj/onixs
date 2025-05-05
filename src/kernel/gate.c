#include <onixs/gate.h>
#include <onixs/syscall.h>
#include <onixs/task.h>
#include <onixs/console.h>
#include <onixs/memorry.h>
#include <onixs/clock.h>
#include <onixs/device.h>
#include <onixs/buffer.h>

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
    char ch;
    device_t *device = device_find(DEV_IDE_DISK, 0);
    assert(device);
    buffer_t *buf = bread(device->dev, 0);
    char *data = buf->data + SECTOR_SIZE;
    memset(data, 0x5A, SECTOR_SIZE);
    buf->dirty = true;
    brelse(buf);
    return 255;
}

static int32 sys_write(fd_t fd, char * buf, uint32 len)
{
    if(fd == stdout || fd == stderr)
    {
        return console_write(NULL, buf, len);
    }
    panic("write");
    return 0;
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
    syscall_table[SYS_NR_WRIET] = sys_write;
    syscall_table[SYS_NR_WAITPID] = task_waitpid;
    syscall_table[SYS_NR_TIME] = sys_time;
    syscall_table[SYS_NR_GETPID] = sys_getpid;
    syscall_table[SYS_NR_GETPPID] = sys_getppid;
    syscall_table[SYS_NR_BRK] = sys_brk;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;
    syscall_table[SYS_NR_FORK] = task_fork;
}