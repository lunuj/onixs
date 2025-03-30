#ifndef MUTEX_H
#define MUTEX_H

#include <onixs/list.h>
#include <onixs/types.h>

typedef struct mutex_t {
    bool value;
    list_t waiters;
} mutex_t;

void mutex_init(mutex_t * mutex);
void mutex_lock(mutex_t * mutex);
void mutex_unlock(mutex_t * mutex);


typedef struct spinlock_t {
    struct task_t *holder;
    mutex_t mutex;
    uint32 repeat;
}spinlock_t;

void spin_init();
void spin_lock();
void spin_unlock();
#endif // MUTEX_H