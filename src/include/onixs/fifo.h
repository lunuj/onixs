#ifndef FIFO_H
#define FIFO_H
#include <onixs/types.h>

typedef struct fifo_t
{
    char * buf;
    uint32 length;
    uint32 head;
    uint32 tail;
}fifo_t;

void fifo_init(fifo_t * fifo, char * buf, uint32 length);
bool fifo_full(fifo_t * fifo);
bool fifo_empty(fifo_t * fifo);
char fifo_get(fifo_t * fifo);
void fifo_put(fifo_t * fifo, char byte);


#endif // FIFO_H
