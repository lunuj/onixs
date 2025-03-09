#ifndef TASK_H
#define TASK_H

#include <onixs/types.h>
typedef uint32 target_t();
typedef struct task_t{
    uint32 *stack;
} task_t;

typedef struct task_frame_t
{
    uint32 edi;
    uint32 esi;
    uint32 ebx;
    uint32 ebp;
    uint32 (*eip)(void);
}task_frame_t;
void task_init();

#endif // TASK_H


