#ifndef STDARG_H
#define STDARG_H

typedef char *va_list;
#define va_start(ap, v) (ap = (char *)&v + sizeof(char *))
#define va_arg(ap, t) (*(t *)((ap += sizeof(char *)) - sizeof(char *)))
#define va_end(ap) (ap = (va_list)0);

#endif // STDARG_H