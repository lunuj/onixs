#include <onixs/fs.h>
#include <onixs/syscall.h>

int main(int argc, char *argv[])
{
    char ch;
    while (true)
    {
        int n = read(STDIN_FILENO, &ch, 1);
        if(n == EOF)
            break;
        if(ch == '\n')
        {
            write(STDOUT_FILENO, &ch, 1);
            break;
        }
        write(STDOUT_FILENO, &ch, 1);
        write(STDOUT_FILENO, &ch, 1);
    }
    return 0;
}