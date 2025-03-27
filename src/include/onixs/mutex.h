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

#endif // MUTEX_H