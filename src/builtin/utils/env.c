#include <onixs/types.h>
#include <onixs/stdio.h>
#include <onixs/syscall.h>
#include <onixs/string.h>

int main(int argc, char const *argv[], char const *envp[])
{
    printf("hello world\n");
    for (size_t i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }

    for (size_t i = 0; 1; i++)
    {
        printf("%s\n", envp[i]);
        if (!envp[i])
            break;
        int len = strlen(envp[i]);
        if (len >= 1022)
            continue;
    }
    return 0;
}
