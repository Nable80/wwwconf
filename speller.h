/***************************************************************************
                          speller.h  -  spelling, ban, html cheker header
                             -------------------
    begin                : Mon Mar 19 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef SPELLER_H_INCLUDED
#define SPELLER_H_INCLUDED

#include "basetypes.h"
#include "profiles.h"
#include "dbase.h"

/* error codes for CheckSpellingBan() */
typedef enum {
        MSG_CHK_ERROR_PASSED = 1,
        MSG_CHK_ERROR_NONAME = 2,
        // MSG_CHK_ERROR_NOEMAIL = 3,
        MSG_CHK_ERROR_NOMSGHEADER = 4,
        MSG_CHK_ERROR_NOMSGBODY = 5,
        MSG_CHK_ERROR_BADSPELLING = 6,
        MSG_CHK_ERROR_BANNED = 7,
        MSG_CHK_ERROR_CLOSED = 8,
        MSG_CHK_ERROR_INVALID_NUMBER = 9,
        MSG_CHK_ERROR_INVALID_PASSW = 10,
        MSG_CHK_ERROR_ROBOT_MESSAGE = 11,
        MSG_CHK_ERROR_EDIT_DENIED = 12,
        MSG_CHK_ERROR_INVISIBLE = 13
} msg_chk_error_t;

#define PROFILE_CHK_ERROR_ALLOK                                        1
#define PROFILE_CHK_ERROR_ALREADY_EXIST                        2
#define PROFILE_CHK_ERROR_NOT_EXIST                                3
#define PROFILE_CHK_ERROR_INVALID_LOGIN_SPELL        4
#define PROFILE_CHK_ERROR_INVALID_PASSWORD                5
#define PROFILE_CHK_ERROR_INVALID_PASSWORD_REP        6
#define PROFILE_CHK_ERROR_SHORT_PASSWORD                7
#define PROFILE_CHK_ERROR_INVALID_EMAIL                        8
#define PROFILE_CHK_ERROR_CANNOT_DELETE_USR                9
#define PROFILE_CHK_ERROR_UNKNOWN_ERROR                        10

#define SPELLER_FILTER_HTML                                                0x0002
#define SPELLER_PARSE_TAGS                                                0x0001

#define ALL_ENCODE                0x0000  //all but a-A z-Z 0-9
#define URL_ENCODE                0x0001   //allow '~','-','_','/'
#define MAIL_ENCODE                0x0002

/* bit mask of CFlags format in CheckSpellingBan() */
#define MSG_CHK_DISABLE_WWWCONF_TAGS                        0x0001
#define MSG_CHK_DISABLE_SMILE_CODES                                0x0002
#define MSG_CHK_ENABLE_EMAIL_ACKNL                                0x0004
#define MSG_CHK_ALLOW_HTML                                                0x0008
#define MSG_CHK_DISABLE_SIGNATURE                                0x0010

#define SPELLER_INTERNAL_BUFFER_SIZE                        10000

/* code string to http format, if allocmem = 1 - function will allocate memory for you,
 * otherwise internal buffer will be used (10K buffer) */
char* CodeHttpString(char *s, int allocmem = 1, int type = ALL_ENCODE);


/* check email (with current #define settings) */
int IsMailCorrect(char *s);

/* filter html tags, if allocmem = 1 - function will allocate memory for you,
 * otherwise internal buffer will be used (10K buffer) */
char* FilterHTMLTags(const char *s, size_t ml, int allocmem = 1);

/* filter unicode bidirectional markers */
char* FilterBiDi(const char *s);

char* FilterWhitespaces(char *s);

/* prepare every text in this board to be printed to browser */
int PrepareTextForPrint(char *msg, char **res, BYTE index, DWORD flags);

/* check message for correct and check HTML Tags, bad words list, and banned user */
int CheckSpellingBan(struct SMessage *mes, char **body, char **Reason,
                                         DWORD CFlags, DWORD *RetFlags, bool fRegged = true);

/* replace ascii control characters (1-31, 127) by a space */
char *FilterControl(const char *s);

/* filter data to be able to place it between '<![CDATA[' and ']]>' */
char *FilterCdata(const char *s);

/* Decode escape sequences using by FilterHTMLtags() */
char *DefilterHTMLTags(const char *s);

#endif
