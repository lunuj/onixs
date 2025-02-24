#ifndef TYPES_H
#define TYPES_H

#define EOF -1
#define EOS '\0'
#define NULL ((void *)0)
#define bool _Bool
#define true 1
#define false 0
#define _packed __attribute__((packed))
#define _ofp __attribute__((optimize("omit-frame-pointer")))
typedef unsigned int size_t;
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;


#endif // TYPES_H