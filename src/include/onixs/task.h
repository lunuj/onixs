#ifndef TASK_H
#define TASK_H

#include <onixs/types.h>
#include <onixs/bitmap.h>
#include <onixs/list.h>

#define KERNEL_USER 0
#define NORMAL_USER 1

#define TASK_NAME_LEN 16
#define TASK_NUMBER 64

typedef enum task_state_t{
    TASK_INIT,          // 创建
    TASK_RUNNING,       // 运行
    TASK_READY,         // 就绪
    TASK_BLOCKED,       // 阻塞
    TASK_SLEEPING,      // 睡眠
    TASK_WAITTING,      // 等待
    TASK_DIED,          // 结束
} task_state_t;

typedef uint32 target_t();

typedef struct task_t{
    uint32 *stack;                  // 内核栈
    list_node_t node;               // 任务阻塞节点
    task_state_t state;             // 任务状态
    uint32 priority;                // 任务优先级
    uint32 ticks;                   // 剩余时间片
    uint32 jiffies;                 // 上次执行时全局时间片
    char name[TASK_NAME_LEN];       // 任务名
    uint32 uid;                     // 用户id
    uint32 pde;                     // 页目录物理地址
    struct bitmap_t * vmap;         // 进程虚拟内存位图
    uint32 magic;                   // 内核魔术用于检测栈溢出
} task_t;

typedef struct task_frame_t
{
    uint32 edi;
    uint32 esi;
    uint32 ebx;
    uint32 ebp;
    uint32 (*eip)(void);
}task_frame_t;

extern bitmap_t kernel_map;

void task_init();
void schedule();
task_t * running_task();
void task_block(task_t * task, list_t * blist, task_state_t state);
void task_unblock(task_t * task);
extern void task_switch(task_t *next);

#endif // TASK_H


