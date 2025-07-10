#include <onixs/stdlib.h>

void delay(uint32 count){
    while (count--);
}

void hang(bool statu){
    while(statu);
}

uint8 bcd_to_bin(uint8 value){
    return (value & 0xF) + (value >> 4)*10;
}

uint8 bin_to_bcd(uint8 value){
    return (value / 10) * 0x10 + (value % 10);
}

uint32 div_round_up(uint32 num, uint32 size)
{
    return (num + size - 1) / size;
}

int atoi(const char *str)
{
    if(str == NULL)
        return 0;
    bool sign = 1;
    int result = 0;
    if(*str == '-')
    {
        sign = -1;
        str++;
    }
    for (; *str; str++)
    {
        result = result * 10 + (*str - '0');
    }
    return result;
}