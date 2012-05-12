#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/file.h>
#include <errno.h>

#include "basetypes.h"
#include "n_error.h"
#include "n_log.h"

int www_log_level;

#define IDENT  "wwwconf"
#define LOGFILE  "www.log"
#define FACILITY  LOG_LOCAL0

// %s:%d - for $file:$line
// "[$date] [$level] [$pid] [%s:%d] $msg\n"
#define DATE_LEN  24 // strlen(ctime(time)) w/o trailing \n
#define LEVEL_STR_LEN  7  // max of strlen(strs[i]) (see add_prefix()'s body)
#define PID_LEN  (sizeof(long)*8/3)
#define SRC_SPEC_LEN  5 // strlen(%s:%d)
// 3 is strlen("[] "), 1 for '\n'
#define ACTUAL_FORMAT_LEN(orig_len)                                       \
        (DATE_LEN + LEVEL_STR_LEN + PID_LEN + SRC_SPEC_LEN + 3*4 + orig_len + 1)

#define FORMAT_LEN  ACTUAL_FORMAT_LEN(WWW_LOG_FORMAT_LEN)

#define LOG_FORMAT  "%s"
#define LOCAL_LOG_FORMAT_LEN  2  // strlen("%s")
#define LOG_FORMAT_LEN  ACTUAL_FORMAT_LEN(LOCAL_LOG_FORMAT_LEN)

static int add_prefix(int level, const char *in, char *out, size_t len);

static int log2file(const char *format, ...)
        __attribute__ ((format(printf, 1, 2)));
static int vlog2file(const char *format, va_list ap);
static void log2sys(int level, const char *format, ...)
        __attribute__ ((format(printf, 2, 3)));
static void vlog2sys(int level, const char *format, va_list ap);

static int lock_routine(FILE *f, int op);
#define LOCK(f)  lock_routine(f, LOCK_EX)
#define UNLOCK(f)  lock_routine(f, LOCK_UN)

static void local_log(int level, const char *file, int line, const char *msg);
#define LOCAL_LOG(level, msg)  local_log(level, __FILE__, __LINE__, msg)
#define ERR(msg)  LOCAL_LOG(WWW_LOG_LEVEL_ERR, msg)
#define SYSERR()  LOCAL_LOG(WWW_LOG_LEVEL_ERR, strerror(errno))
#define WARNING(msg)  LOCAL_LOG(WWW_LOG_LEVEL_WARNING, msg)

static const int sys_level[WWW_LOG_LEVEL_OFF] = {
        /* [WWW_LOG_LEVEL_DEBUG  ] = */  LOG_DEBUG,
        /* [WWW_LOG_LEVEL_INFO   ] = */  LOG_INFO,
        /* [WWW_LOG_LEVEL_NOTICE ] = */  LOG_NOTICE,
        /* [WWW_LOG_LEVEL_WARNING] = */  LOG_WARNING,
        /* [WWW_LOG_LEVEL_ERR    ] = */  LOG_ERR,
        /* [WWW_LOG_LEVEL_CRIT   ] = */  LOG_CRIT,
        /* [WWW_LOG_LEVEL_ALERT  ] = */  LOG_ALERT,
        /* [WWW_LOG_LEVEL_EMERG  ] = */  LOG_EMERG
};

void www_log(int level, const char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        www_vlog(level, format, ap);
        va_end(ap);
}

void www_vlog(int level, const char *format, va_list ap)
{
        char prefix_format[FORMAT_LEN];
        int res;

        if (level < www_log_level)
                return;

        res = add_prefix(level, format, prefix_format, FORMAT_LEN);

        if (res < 0) {
                SYSERR();
                ERR("the format string is too long");
                ERR("unable to write to the log file '" LOGFILE "'");
                return;
        } else if ((unsigned) res >= FORMAT_LEN)
                WARNING("the format string was too long and have been cut");

        if (vlog2file(prefix_format, ap)) {
                ERR("unable to write to the log file '" LOGFILE "'");
                vlog2sys(level, prefix_format, ap);
        }
}

int add_prefix(int level, const char *in, char *out, size_t len)
{
        static const char *strs[WWW_LOG_LEVEL_OFF] = {
                /* [WWW_LOG_LEVEL_DEBUG  ] = */  "debug",
                /* [WWW_LOG_LEVEL_INFO   ] = */  "info",
                /* [WWW_LOG_LEVEL_NOTICE ] = */  "notice",
                /* [WWW_LOG_LEVEL_WARNING] = */  "warning",
                /* [WWW_LOG_LEVEL_ERR    ] = */  "error",
                /* [WWW_LOG_LEVEL_CRIT   ] = */  "critical",
                /* [WWW_LOG_LEVEL_ALERT  ] = */  "alert",
                /* [WWW_LOG_LEVEL_EMERG  ] = */  "panic"
        };

        time_t t;
        char *date;

        t = time(NULL) + DATETIME_DEFAULT_TIMEZONE*3600;
        date = asctime(gmtime(&t));
        date[DATE_LEN] = '\0';

        return snprintf(out, len, "[%s] [%s] [%ld] [%%s:%%d] %s\n",
                           date, strs[level], (long) getpid(), in);
}

void local_log(int level, const char *file, int line, const char *msg)
{
        static char format[LOG_FORMAT_LEN];
        add_prefix(level, LOG_FORMAT, format, LOG_FORMAT_LEN);
        log2sys(sys_level[level], format, file, line, msg);
}

int log2file(const char *format, ...)
{
        int res;
        va_list ap;

        va_start(ap, format);
        res = vlog2file(format, ap);
        va_end(ap);
        
        return res;
}

int vlog2file(const char *format, va_list ap)
{
        static FILE *f;

        if (f == NULL && (f = fopen(LOGFILE, "a")) == NULL) {
                SYSERR();
                goto fail;
        }
        if (LOCK(f))
                goto fail_close;
        if (vfprintf(f, format, ap) < 0) {
                SYSERR();
                goto fail_unlock;
        }
        if (UNLOCK(f))
                goto fail_close;

        return 0;

 fail_unlock:
        UNLOCK(f);
 fail_close:
        if (f) {
                fclose(f);
                f = NULL;
        }
 fail:
        return -1;
}

void log2sys(int level, const char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        vlog2sys(level, format, ap);
        va_end(ap);
}

void vlog2sys(int level, const char *format, va_list ap)
{
        static int toinit = 1;
        
        if (toinit) {
                toinit = 0;
                openlog(IDENT, 0, FACILITY);
        }

        vsyslog(sys_level[level], format, ap);
}

int lock_routine(FILE *f, int op)
{
        int fd;

        if (fflush(f)) {
                SYSERR();
                goto fail;
        }
        if ( (fd = fileno(f)) == -1) {
                SYSERR();
                goto fail;
        }
        if (flock(fd, op)) {
                SYSERR();
                goto fail;
        }

        return 0;

 fail:
        return -1;
}
