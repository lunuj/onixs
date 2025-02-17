#ifndef STRING_H
#define STRING_H

#include <onixs/types.h>

#define SMALL       0x01
#define ZEROPAD     0x02
#define LEFT        0x04
#define SIGN        0x08
#define PLUS        0x10
#define SPECIAL     0x20
#define SPACE       0x40

#define is_digit(c) ((c) >= '0' && (c) <= '9')

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t count);
char *strcat(char *dest, const char *src);
size_t strlen(const char *str);
size_t strnlen(const char *str, size_t maxlen);
int strcmp(const char *lhs, const char *rhs);
char *strchr(const char *str, int ch);
char *strrchr(const char *str, int ch);
char *strsep(const char *str);
char *strrsep(const char *str);

int atoi(const char *s);
char * number(char *str, uint32 num, int base, int size, int precision, int flags);

int memcmp(const void *lhs, const void *rhs, size_t count);
void *memset(void *dest, int ch, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memchr(const void *ptr, int ch, size_t count);

#endif // STRING_H