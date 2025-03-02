#ifndef STDLIB_H
#define STDLIB_H

#include <onixs/types.h>
void delay(uint32 count);
void hang(bool statu);
uint8 bcd_to_bin(uint8 value);
uint8 bin_to_bcd(uint8 value);

#endif // STDLIB_H