#ifndef STDLIB_H
#define STDLIB_H

#include <onixs/types.h>

#define MAX(a, b) (a<b ? b : a)
#define MIN(a, b) (a<b ? a : b)

void delay(uint32 count);
void hang(bool statu);
uint8 bcd_to_bin(uint8 value);
uint8 bin_to_bcd(uint8 value);
uint32 div_round_up(uint32 num, uint32 size);
#endif // STDLIB_H