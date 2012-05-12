#ifndef N_LOG_H
#define N_LOG_H

extern int www_loglevel;

enum {
        WWW_LOG_LEVEL_DEBUG,
        WWW_LOG_LEVEL_INFO,
        WWW_LOG_LEVEL_NOTICE,
        WWW_LOG_LEVEL_WARNING,
        WWW_LOG_LEVEL_ERR,
        WWW_LOG_LEVEL_CRIT,
        WWW_LOG_LEVEL_ALERT,
        WWW_LOG_LEVEL_EMERG,
        WWW_LOG_LEVEL_OFF
};

void www_log(int level, const char *fmt, ...)
        __attribute__ ((format(printf, 2, 3)));
void www_vlog(int level, const char *fmt, va_list ap);

#define WWW_LOG_FORMAT_LEN  1000

#define WWW_LOG_AT(level, format, ...)  www_log(level, format, __FILE__, __LINE__, ##__VA_ARGS__)

#define WWW_LOG_DEBUG(...)    WWW_LOG_AT(WWW_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define WWW_LOG_INFO(...)     WWW_LOG_AT(WWW_LOG_LEVEL_INFO, __VA_ARGS__)
#define WWW_LOG_NOTICE(...)   WWW_LOG_AT(WWW_LOG_LEVEL_NOTICE, __VA_ARGS__)
#define WWW_LOG_WARNING(...)  WWW_LOG_AT(WWW_LOG_LEVEL_WARNING, __VA_ARGS__)
#define WWW_LOG_ERR(...)      WWW_LOG_AT(WWW_LOG_LEVEL_ERR, __VA_ARGS__)
#define WWW_LOG_CRIT(...)     WWW_LOG_AT(WWW_LOG_LEVEL_CRIT, __VA_ARGS__)
#define WWW_LOG_ALERT(...)    WWW_LOG_AT(WWW_LOG_LEVEL_ALERT, __VA_ARGS__)
#define WWW_LOG_EMERG(...)    WWW_LOG_AT(WWW_LOG_LEVEL_EMERG, __VA_ARGS__)

#endif
