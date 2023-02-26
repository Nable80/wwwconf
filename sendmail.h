/***************************************************************************
                          sendmail.h  -  mail sending support include
                             -------------------
    begin                : Wed Jun 13 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef SENDMAIL_H_INCLUDED
#define SENDMAIL_H_INCLUDED

int wcSendMail(const char *to, const char *subj, const char *body);

#endif
