/***************************************************************************
                          boardtags.h  -  board tags support include
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef BOARDTAGS_H_INCLUDED
#define BOARDTAGS_H_INCLUDED

#include "basetypes.h"

#define WC_TAG_OPEN   '['
#define WC_TAG_CLOSE  ']'

#define TRY_AUTO_URL_PREPARSE        1
#define PARSED_URL_TMPL                "<A HREF=\"%s\" STYLE=\"text-decoration:underline;\" TARGET=_blank>%s</A>"

#define MAX_NESTED_TAGS                        8

#define BOARDTAGS_EXPAND_XMLEN  0x04000000
#define BOARDTAGS_CUT_TAGS      0x08000000
#define BOARDTAGS_TAG_PREPARSE  0x10000000
#define BOARDTAGS_EXPAND_ENTER  0x20000000
#define BOARDTAGS_PURL_ENABLE   0x40000000

#define BoardTagCount  19
#define BoardPicCount  46

#define RED_TAG_TYPE    5
#define COLOR_TAG_TYPE  6
#define URL_TAG_TYPE    7
#define PIC_TAG_TYPE    8
#define Q_TAG_TYPE      10
#define PRE_TAG_TYPE    12
#define TEX_TAG_TYPE    16
#define TUB_TAG_TYPE    17

#define WC_TAG_TYPE_DISABLED  0
#define WC_TAG_TYPE_1         1
#define WC_TAG_TYPE_2         2
#define WC_TAG_TYPE_12        3
#define WC_TAG_TYPE_ONLYOPEN  4

/* element of table for converting WWWConf Tags to HTML */
struct STagConvert {
        /* board tag */
        const char *tag;
        /* corresponding opening and closing HTML tags */
        const char *topentag;
        const char typeopen;
        const char *tclosetag;
        const char typeclose;
        /* if tag is allowed in a subject and an altname
         * as they are printed in index
         */
        BYTE index;
        const char *topentag2;
        const char *tclosetag2;
};

struct SPicConvert {
        /* board code */
        const char *tag;
        /* Pic URL */
        const char *url;
};

/* Struct for saving last opened tag */
struct SSavedTag {
        /* tag type and tag length */
        int tt, tl;        
        /* tag old and expanded expression */
        char *tagexp;
        char *oldexp;
        /* insert expanded expression index */
        int index;
};

int FilterBoardTags(char *s, char **r, BYTE index, DWORD ml, DWORD Flags, DWORD *RF);

#endif
