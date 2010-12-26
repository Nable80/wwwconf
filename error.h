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

void print2log(const char *s, ...);
int printwidehtmlerror(const char *file, DWORD line, const char *s);

#endif
