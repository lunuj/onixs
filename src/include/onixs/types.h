#ifndef TYPES_H
#define TYPES_H

#define EOF -1
#define EOS '\0'

#define CONCAT(x, y) x##y
#define RESERVED_TOKEN(x, y) CONCAT(x, y)
#define RESERVED RESERVED_TOKEN(reserved, __LINE__)

#define NULL ((void *)0)

#define bool _Bool
#define true 1
#define false 0

#define _packed __attribute__((packed))
#define _ofp __attribute__((optimize("omit-frame-pointer")))
#define _inline __attribute__((always_inline)) inline

typedef unsigned int size_t;
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef uint32 time_t;
typedef uint32 idx_t;
typedef uint32 dev_t;
typedef uint16 mode_t; // 文件权限

typedef int32 fd_t;
typedef int32 pid_t;
typedef int32 off_t;
typedef enum std_fd_t
{
    STDIN_FILENO,
    STDOUT_FILENO,
    STDERR_FILENO,
}std_fd_t;

#endif // TYPES_H