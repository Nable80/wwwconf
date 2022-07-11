/***************************************************************************
                          sendmail.h  -  mail sending support include
                             -------------------
    begin                : Wed Jun 13 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef SENDMAIL_H_INCLUDED
#define SENDMAIL_H_INCLUDED

#include "basetypes.h"
#include "messages.h"
// from base64.h

#define MAIL_SEND_HELO "HELO localhost\r\nMAIL FROM: <%s>\r\nRCPT TO: <%s>\r\nDATA\r\n"
#define MAIL_SEND_DATA "From: <%s>\r\nTo: <%s>\r\nSubject:=?Windows-1251?B?%s?=\r\n"\
        MAILACKN_HEADER  "%s" MAIL_SEND_SIGNING "\r\n.\r\nQuit\r\n"
                
int wcSendMail(char *to, char *subj, char *body);

struct buffer_st {
  char *data;
  size_t length;
  char *ptr;
  size_t offset;
};

void buffer_new(struct buffer_st *b);
void buffer_add(struct buffer_st *b, char c);
void buffer_delete(struct buffer_st *b);

void base64_encode(struct buffer_st *b, const char *source, size_t length);
#endif
