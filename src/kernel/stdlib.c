#include <onixs/stdlib.h>

void delay(uint32 count){
    while (count--);
}

void hang(bool statu){
    while(statu);
}