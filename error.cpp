/***************************************************************************
                          error.cpp  -  log&error handler
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/
#include <cstdio>

#include "basetypes.h"
#include "stdarg.h"
#include "dbase.h"
#include "error.h"
#include "messages.h"

int error_type;

/* print message to logfile */
__attribute__ ((format (printf, 1, 2)))
void print2log(const char *format, ...)
{
        FILE *f;
        time_t t;
        char time_prefix[sizeof("YYYY-mm-dd HH:MM:SS : ")] = "";

        if ((f = fopen(LOG_FILE, "a")) != NULL) {
                // try to ensure that our write will be atomic
                lock_file(f);

                // print time prefix
                t = time(NULL) + 3600 * DATETIME_DEFAULT_TIMEZONE;
                strftime(time_prefix, sizeof(time_prefix), "%Y-%m-%d %H:%M:%S : ", gmtime(&t));
                fputs(time_prefix, f);

                // then the actual log message
                va_list ap;
                va_start(ap, format);
                vfprintf(f, format, ap);
                va_end(ap);

                // and finally a terminating '\n'
                fputc('\n', f);

                // flush and release resources
                unlock_file(f);
                fclose(f);
        }
}

/* print error to file [file] at line [line] and message then immediately exit */
[[ noreturn ]] void printwidehtmlerror(const char *file, DWORD line, const char *s, const char *p)
{
        // handle error messages with a parameter:
        char *ss = NULL;
        if (s && p && asprintf(&ss, s, p) >= 0) {
                s = ss;
        }

#if ENABLE_LOG
        print2log(LOG_UNHANDLED, file, line, getenv(REMOTE_ADDR),
                (s && (*s != 0)) ? s : LOG_ERRORTYPEUNKN, getenv(QUERY_STRING));
#endif

        if (error_type == ERROR_TYPE_XML) {
                printf(XML_START "<error><type>system</type><at>%s:%lu</at>", file, line);
                if (s && *s)
                        printf("<description>%s</description>", s);
                printf("</error>");
                free(ss);
                exit(0);
        } else if (error_type == ERROR_TYPE_XMLFP || error_type == ERROR_TYPE_PLAIN) {
                printf(PLAIN_START "Error at %s:%lu", file, line);
                if (s && *s)
                        printf(":%s.", s);
                free(ss);
                exit(0);
        }

        // for correct showing in browser
        if(!HPrinted) printf("Content-type: text/html\n\n<HTML><HEAD><TITLE>" \
                TITLE_WWWConfBegining TITLE_divider "Error during page refresh</TITLE></HEAD>");

        printf("<p><B><H3>An error occured</H3></B></p><p>Try to refresh this page, and if"
        " this occure again please contact server <a href=\"mailto:%s\">"
        " administrator </a> for support", ADMIN_MAIL);
#if ERROR_ON_SCREEN == 0
#if ENABLE_LOG
        printf("<p><font color=#ff0000> This error have been logged to the"
        " server log");
#endif
#else
        printf("<P><font color=#ff0000><I>Technical details</I></font><BR>" \
                "<font color=#0000ff>" LOG_UNHANDLED_HTML, file, line, getenv(REMOTE_ADDR),
                (s && (*s != 0)) ? s : LOG_ERRORTYPEUNKN, getenv(QUERY_STRING));
#endif
        printf("</HTML>");
        fflush(stdout);
        free(ss);
        exit(0);
}
