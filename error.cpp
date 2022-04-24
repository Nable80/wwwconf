/***************************************************************************
                          error.cpp  -  log&error handler
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "stdarg.h"
#include "dbase.h"
#include "error.h"
#include "messages.h"

int error_type;

/* print message to logfile
 * s - format string
 *                %s                string
 *                %d[f]        DWORD (if 'f' as hex)
 *                %b[f]        BYTE (if 'f' as hex)
 */
void print2log(const char *s, ...)
{
        DWORD i = 0, c = 0;
        FILE *f;
        //
        // POTENTIAL BUG HERE
        //
        char xx[100];
        char *ss = (char*)malloc(500*strlen(s));
        ss[0] = 0;
        va_list marker;
        
/*        if((f = fopen(LOG_FILE, "a")) != NULL) {
                fprintf(f, "entering:");
                fclose(f);
        }*/

        // Initialize variable arguments
        va_start(marker, s);
        DWORD slen = strlen(s);
        while(i < slen)
        {
                if(s[i] == '%') {
                        if(i + 1 < slen) {
                                i++;
                                switch(s[i]) {
                                        case 'd':
                                                {
                                                        bool fhex = false;
                                                        if(i + 1 < slen && s[i+1] == 'f')
                                                        {
                                                                fhex = true;
                                                                i++;
                                                        }
                                                        DWORD x = va_arg(marker, DWORD);
                                                        if(!fhex) sprintf(xx,"%lu", x);
                                                        else {
                                                                sprintf(xx,"%lx", x);
                                                                strcat(ss, "0x");
                                                        }
                                                        strcat(ss, xx);
                                                        break;
                                                }
                                        case 's':
                                                {
                                                        char *x = va_arg(marker, char*);
                                                        if(x) strcat(ss, x);
                                                        else strcat(ss, "null");
                                                        break;
                                                }
                                        case 'b':
                                                {
                                                        bool fhex = false;
                                                        if(i + 1 < slen && s[i+1] == 'f')
                                                        {
                                                                fhex = true;
                                                                i++;
                                                        }
                                                        unsigned x = va_arg(marker, unsigned);
                                                        sprintf(xx, fhex ? "0x%x" : "%u", x & 0xFF);
                                                        strcat(ss, xx);
                                                        break;
                                                }
                                        default:
                                                {
                                                        char tmp[3];
                                                        tmp[0] = s[i-1];
                                                        tmp[1] = s[i];
                                                        tmp[2] = 0;
                                                        strcat(ss, tmp);
                                                }
                                }
                                c = strlen(ss);
                        }
                        else {
                                ss[c] = s[i];
                                c++;
                                ss[c] = 0;
                        }

                }
                else {
                        ss[c] = s[i];
                        c++;
                        ss[c] = 0;
                }
                i++;
        }
        // Reset variable arguments
        va_end(marker);

        time_t t;
        char *p;
        if((f = fopen(LOG_FILE, "a")) != NULL) {
                t = time(NULL);
                t += 3600*DATETIME_DEFAULT_TIMEZONE;
                p = asctime(gmtime(&t));
                p[strlen(p) - 1] = 0;
                fprintf(f, "%s : %s\n", p, ss);
                fclose(f);
        }
        free(ss);
}


/* print error to file [file] at line [line] and message then immediately exit */
[[ noreturn ]] void printwidehtmlerror(const char *file, DWORD line, const char *s)
{
#if ENABLE_LOG
        print2log(LOG_UNHANDLED, file, line, getenv(REMOTE_ADDR),
                (s && (*s != 0)) ? s : LOG_ERRORTYPEUNKN, getenv(QUERY_STRING));
#endif

        if (error_type == ERROR_TYPE_XML) {
                printf(XML_START "<error><type>system</type><at>%s:%lu</at>", file, line);
                if (s && *s)
                        printf("<description>%s</description>", s);
                printf("</error>");
                exit(0);
        } else if (error_type == ERROR_TYPE_XMLFP || error_type == ERROR_TYPE_PLAIN) {
                printf(PLAIN_START "Error at %s:%lu", file, line);
                if (s && *s)
                        printf(":%s.", s);
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
        exit(0);
}
