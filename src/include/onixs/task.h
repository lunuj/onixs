#ifndef TASK_H
#define TASK_H

#include <onixs/types.h>
#include <onixs/bitmap.h>
#include <onixs/list.h>

#define KERNEL_USER 0
#define NORMAL_USER 1000

#define TASK_NAME_LEN 16
#define TASK_NUMBER 64

#define TASK_FILE_NR 16

typedef enum task_state_t{
    TASK_INIT,          // 创建
    TASK_RUNNING,       // 运行
    TASK_READY,         // 就绪
    TASK_BLOCKED,       // 阻塞
    TASK_SLEEPING,      // 睡眠
    TASK_WAITTING,      // 等待
    TASK_DIED,          // 结束
} task_state_t;

typedef void target_t();

typedef struct task_t{
    uint32 *stack;                  // 内核栈
    list_node_t node;               // 任务阻塞节点
    task_state_t state;             // 任务状态
    uint32 priority;                // 任务优先级
    int ticks;                      // 剩余时间片
    uint32 jiffies;                 // 上次执行时全局时间片
    char name[TASK_NAME_LEN];       // 任务名
    uint32 uid;                     // 用户id
    uint32 gid;                     // 用户组 id
    pid_t pid;                      // 任务 id
    pid_t ppid;                     // 副任务 id
    uint32 pde;                     // 页目录物理地址
    struct bitmap_t * vmap;         // 进程虚拟内存位图
    uint32 brk;
    int status;
    pid_t waitpid;
    struct inode_t *ipwd;
    struct inode_t *iroot;
    uint16 umask;                   // 进程用户权限
    struct file_t *files[TASK_FILE_NR];
    uint32 magic;                   // 内核魔术用于检测栈溢出
} task_t;

typedef struct task_frame_t
{
    uint32 edi;
    uint32 esi;
    uint32 ebx;
    uint32 ebp;
    void (*eip)(void);
}task_frame_t;

extern uint32 volatile jiffies;
extern uint32 jiffy;
extern bitmap_t kernel_map;

extern void idle_thread();
extern void init_thread();
extern void test_thread();
extern void task_switch(task_t *next);


// 中断帧
typedef struct intr_frame_t
{
    uint32 vector;

    uint32 edi;
    uint32 esi;
    uint32 ebp;
    // 虽然 pushad 把 esp 也压入，但 esp 是不断变化的，所以会被 popad 忽略
    uint32 esp_dummy;

    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;

    uint32 gs;
    uint32 fs;
    uint32 es;
    uint32 ds;

    uint32 vector0;

    uint32 error;

    uint32 eip;
    uint32 cs;
    uint32 eflags;
    uint32 esp;
    uint32 ss;
} intr_frame_t;

void task_init();

void schedule();

task_t * running_task();

void task_block(task_t * task, list_t * blist, task_state_t state);
void task_unblock(task_t * task);

void task_wakeup();

void task_to_user_mode(target_t target);

fd_t task_get_fd(task_t *task);
void task_put_fd(task_t *task, fd_t fd);

// 系统调用接口
// task.c
void task_exit(int status);
pid_t task_fork();

pid_t task_waitpid(pid_t pid, int * status);

pid_t sys_getpid();
pid_t sys_getppid();

void task_yield();
void task_sleep(uint32 ms);

// system.c
mode_t sys_umask(mode_t mask);

#endif // TASK_H