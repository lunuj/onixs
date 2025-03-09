#ifndef ASSERT_H
#define ASSERT_H

#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
        assert_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

void assert_failure(char *exp, char *file, char *base, int line);
void panic(const char *fmt, ...);
#endif // ASSERT_H