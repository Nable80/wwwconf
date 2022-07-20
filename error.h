/***************************************************************************
                          error.h  -  log&error handler header
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#define printhtmlerror() printwidehtmlerror(__FILE__, __LINE__, "")
#define printhtmlerrormes(s) printwidehtmlerror(__FILE__, __LINE__, s)
#define printhtmlerrorat(p, s) { \
        char ss[10000]; \
        sprintf(ss, p, s); \
        printwidehtmlerror(__FILE__, __LINE__, ss); }

__attribute__ ((format (printf, 1, 2))) void print2log(const char *format, ...);

[[ noreturn ]] void printwidehtmlerror(const char *file, DWORD line, const char *s);

extern int error_type;

#define ERROR_TYPE_HTML   0
#define ERROR_TYPE_XML    1
#define ERROR_TYPE_XMLFP  2
#define ERROR_TYPE_PLAIN  3

#endif
