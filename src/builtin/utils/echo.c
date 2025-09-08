#include <onixs/types.h>
#include <onixs/stdio.h>
#include <onixs/syscall.h>

int main(int argc, char const *argv[])
{
    for (size_t i = 1; i < argc; i++)
    {
        printf(argv[i]);
        if (i < argc - 1)
        {
            printf(" ");
        }
    }
    printf("\n");
    return 0;
}