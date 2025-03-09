#ifndef DEBUG_H
#define DEBUG_H

#define DEBUGK(fmt, args...) debugk(__FILE__, __LINE__, fmt, ##args)
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
void debugk(char *file, int line, const char *fmt, ...);


#endif // DEBUG_H