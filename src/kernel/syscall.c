#include <onixs/syscall.h>

static _inline uint32 _syscall0(uint32 nr)
{
    uint32 ret;
    asm volatile(
        "int $0x80\n"
        : "a="(ret)
        : "a"(nr));
    return ret;
}

uint32 test()
{
    return _syscall0(SYS_NR_TEST);
}

void yield()
{
    _syscall0(SYS_NR_YIELD);
}