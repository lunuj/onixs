#include <onixs/onixs.h>
#include <onixs/types.h>
#include <onixs/io.h>
#include <onixs/console.h>

char message[] = "hello world\n";

void kernel_init(){
    console_clear();
    while(1){
        console_write(message, sizeof(message) - 1);
    }
}