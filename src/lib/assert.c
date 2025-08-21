#include <onixs/assert.h>
#include <onixs/stdarg.h>
#include <onixs/types.h>
#include <onixs/stdio.h>
#include <onixs/string.h>

static uint8 buf[1024];

// 强制阻塞
static void spin(char *name)
{
    printf("spinning in %s ...\n", name);
    while (true);
}
#ifdef ENV
void assert_failure(char *exp, char *file, char *base, int line)
#else
void assertion_failure(char *exp, char *file, char *base, int line)
#endif
{
    printf(
        "\n--> assert(%s) failed!!!\n"
        "--> file: %s \n"
        "--> base: %s \n"
        "--> line: %d \n",
        exp, file, base, line);

    spin("assertion_failure()");

    // 不可能走到这里，否则出错；
    asm volatile("ud2");
}
#ifdef ENV
void panic(const char *fmt, ...)
#else
void paniction(const char *fmt, ...)
#endif
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);

    printf("!!! panic !!!\n--> %s \n", buf);
    spin("panic()");

    // 不可能走到这里，否则出错；
    asm volatile("ud2");
}