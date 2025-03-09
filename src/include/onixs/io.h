#ifndef IO_H
#define IO_H

#include <onixs/types.h>

extern uint8 inb(uint16 port);
extern uint16 inw(uint16 port);

extern void outb(uint16 port, uint8 value);
extern void outw(uint16 prot, uint16 value);

#endif // IO_H