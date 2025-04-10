#ifndef GATE_H
#define GATE_H

#include <onixs/interrupt.h>
#include <onixs/assert.h>
#include <onixs/debug.h>

#define SYSCALL_SIZE 256

void syscall_check(uint32 nr);
void syscall_init();

#endif // GATE_H