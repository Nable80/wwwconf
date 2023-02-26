/***************************************************************************
                          sendmail.cpp  -  mail sending support
                             -------------------
    begin                : Mon Nov 11 2002
    copyright            : (C) 2002 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "sendmail.h"
#include "messages.h"
#include "error.h"

static const char dtable[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

struct buffer_st {
  char *data;
  size_t length;
  char *ptr;
  size_t offset;
};

static void buffer_new(struct buffer_st *b)
{
  b->length = 512;
  b->data = (char *)malloc(b->length);
  b->ptr = b->data;
  b->offset = 0;
}

static void buffer_add(struct buffer_st *b, char c)
{
  *(b->ptr++) = c;
  b->offset++;
  if (b->offset == b->length) {
    b->length += 512;
    b->data = (char *)realloc(b->data, b->length);
    b->ptr = b->data + b->offset;
  }
}

static void buffer_delete(struct buffer_st *b)
{
  free(b->data);
  b->length = 0;
  b->offset = 0;
  b->ptr = NULL;
  b->data = NULL;
}

static void base64_encode(struct buffer_st *b, const char *source, size_t length)
{
  int i, hiteof = 0;
  size_t offset = 0;

  while (!hiteof) {
    unsigned char igroup[3];
    char ogroup[4];
    int c, n;

    igroup[0] = igroup[1] = igroup[2] = 0;
    for (n = 0; n < 3; n++) {
      c = *(source++);
      offset++;
      if (offset > length) {
        hiteof = 1;
        break;
      }
      igroup[n] = (unsigned char) c;
    }
    if (n > 0) {
      ogroup[0] = dtable[igroup[0] >> 2];
      ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
      ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
      ogroup[3] = dtable[igroup[2] & 0x3F];

      /* Replace characters in output stream with "=" pad
         characters if fewer than three characters were
         read from the end of the input stream. */

      if (n < 3) {
        ogroup[3] = '=';
        if (n < 2) {
          ogroup[2] = '=';
        }
      }
      for (i = 0; i < 4; i++) {
        buffer_add(b, ogroup[i]);
        //if (!(b->offset % 72)) {
        //  buffer_add(b, '\r');
        //  buffer_add(b, '\n');
        //}
      }
    }
  }
   //buffer_add(b, '\r');
  //buffer_add(b, '\n');
  buffer_add(b, '\0');
}

int wcSendMail(const char *to, const char *subj, const char *body)
{
        int result = 0;
        struct buffer_st subj_base64;
        FILE* sendmail_pipe;

        const char *mail_format =
                "From: <" MA_FROM ">\n"
                "To: <%s>\n"
                "Subject: =?Windows-1251?B?%s?=\n"
                "Content-type: text/html; charset=\"windows-1251\"\n"
                "\n"
                "%s" MAIL_SEND_SIGNING "\n"
                ".\n";

        buffer_new(&subj_base64);
        if (!subj_base64.data) {
                return 0;
        }
        base64_encode(&subj_base64, subj, strlen(subj));

        if ((sendmail_pipe = popen(MA_SENDER, FILE_ACCESS_MODES_W))) {
                fprintf(sendmail_pipe, mail_format, to, subj_base64.data, body);
                if (pclose(sendmail_pipe) == 0)
                        result = 1;
        }

        buffer_delete(&subj_base64);
        return result;
}
