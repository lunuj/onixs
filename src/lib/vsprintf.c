#include <onixs/stdarg.h>
#include <onixs/string.h>
#include <onixs/assert.h>

int vsprintf(char *buf, const char *fmt, va_list args){
    char *str;
    char qualifier;
    int flags, field_width, precision;
    for(str = buf;*fmt;++fmt){
        if(*fmt!='%'){
            *str++ = *fmt;
            continue;
        }
        flags = 0;
    repeat:
        ++fmt;
        switch (*fmt)
        {
            case '-':
                flags |= LEFT;
                goto repeat;
            case '+':
                flags |= PLUS;
                goto repeat;
            case ' ':
                flags |= SPACE;
                goto repeat;
            case '#':
                flags |= SPECIAL;
                goto repeat;
            case '0':
                flags |= ZEROPAD;
                goto repeat;
            default:
                break;
        }

        field_width = -1;
        if(is_digit(*fmt)){
            field_width = skip_atoi(&fmt);
        }
        else if(*fmt == '*'){
            ++fmt;
            field_width = va_arg(args, int);
            if(field_width < 0){
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        precision = -1;
        if(*fmt == '.'){
            ++fmt;
            if(is_digit(*fmt)){
                precision = skip_atoi(&fmt);
            }
            else if(*fmt == '*'){
                precision = va_arg(args, int);
            }
            if(precision < 0){
                precision = 0;
            }
        }

        qualifier = -1;
        if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L'){
            qualifier = *fmt;
            ++fmt;
        }

        switch (*fmt)
        {
            case 'c':
                if(!(flags & LEFT))
                    while (--field_width > 0)
                        *str++=' ';
                *str++=va_arg(args, int);
                while (--field_width > 0)
                    *str++=' ';
                break;
            case 's':
                char * s = va_arg(args, char *);
                int len = strlen(s);
                if(0 > precision){
                    precision = len;
                }else if(len > precision){
                    len = precision;
                }
                if(!(flags & LEFT))
                    while (--field_width > 0)
                        *str++=' ';
                for(int i = 0; i < len; i++)
                    *str++ = *s++;
                while (--field_width > 0)
                    *str++=' ';
                break;
            case 'o':
                str = number(str, va_arg(args, int), 8, field_width, precision, flags);
                break;
            case 'p':
                if(field_width == -1){
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
                break;
            case 'x':
                flags |= SMALL;
            case 'X':
                str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
                break;
            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
                break;
            case 'n':
                int * ip = va_arg(args, int *);
                *ip = str - buf;
                break;
            default:
                if(*fmt != '%'){
                    *str++='%';
                }
                if(*fmt){
                    *str++=*fmt;
                }else
                    --fmt;
                break;
        }
    }
    *str = '\0';
    int i = str - buf;
    assert(i < 1024);
    return i;
}

int sprintf(char *buf, const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}