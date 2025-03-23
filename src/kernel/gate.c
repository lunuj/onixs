#include <onixs/gate.h>
#include <onixs/syscall.h>
#include <onixs/task.h>

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
    static task_t * task;
    if(!task){
        task = running_task();
        task_block(task, NULL, TASK_BLOCKED);
    }else{
        task_unblock(task);
        task = NULL;
    }
    return 255;
}

static void sys_yield(){
    // LOGK("syscall yield...\n");
    schedule();
}

void syscall_init(){
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = sys_default;
    }
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_YIELD] = sys_yield;
}