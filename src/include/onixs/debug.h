#ifndef DEBUG_H
#define DEBUG_H

#define LOG_FATAL   0
#define LOG_ERROR   1
#define LOG_WARN    2
#define LOG_INFO    3
#define LOG_DEBUG   4
#define LOG_TRACE   5

#define LOG_LV      1
#define LOG_MAX_LV  5

#define DEBUGK(fmt, args...) debugk(__FILE__, __LINE__, fmt, ##args)
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
#define SYS_LOG(lv, fmt, args...)               \
    if(lv <= LOG_LV)                            \
    {                                           \
        logk(lv, fmt, ##args);                  \
    }                                           \

void debugk(char *file, int line, const char *fmt, ...);
void logk(int lv, const char *fmt, ...);


#endif // DEBUG_H