/***************************************************************************
                          boardtags.cpp  -  board tags support
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "boardtags.h"
#include "error.h"

/* following define WWWConf Board Tags, and
 * it's translation into standart HTML tags,
 * and security level of each tag
 */
STagConvert TagConvTable[BoardTagCount] = {
        /* text output tags */
        {"B", "<B>", WC_TAG_TYPE_1, "</B>", WC_TAG_TYPE_1, 10},
        {"I", "<I>", WC_TAG_TYPE_1, "</I>", WC_TAG_TYPE_1, 10},
        {"U", "<U>", WC_TAG_TYPE_1, "</U>", WC_TAG_TYPE_1, 10},
        {"H", "<B><FONT SIZE=4>", WC_TAG_TYPE_1, "</FONT></B>", WC_TAG_TYPE_1, 10},
        {"S", "<FONT SIZE=1>", WC_TAG_TYPE_1, "</FONT>", WC_TAG_TYPE_1, 10},
        /* Colour tags */
        {"RED", "<span class=\"btc\" style=\"color:#FF0000\">", WC_TAG_TYPE_1, "</span>", WC_TAG_TYPE_1, 10},
        {"COLOR", "<span class=\"btc\" style=\"color:#%s\">", WC_TAG_TYPE_2, "</span>", WC_TAG_TYPE_1, 10},
        {"C", "<span  class=\"btc\" style=\"color:#%s\">", WC_TAG_TYPE_2, "</span>", WC_TAG_TYPE_1, 10},
        /* URL, Picture, Mail */
        {"URL", "<A HREF=\"%s\" STYLE=\"text-decoration:underline;\" TARGET=_blank>", WC_TAG_TYPE_2, "</A>", WC_TAG_TYPE_1, 10},
        {"PIC", "<IMG class=\"imgtag\" src=\"", WC_TAG_TYPE_1, "\">", WC_TAG_TYPE_1, 10},
        {"MAIL", "<A HREF=\"mailto:%s\" STYLE=\"text-decoration:underline;\">", WC_TAG_TYPE_2, "</A>", WC_TAG_TYPE_1, 10},
        /* HR SIZE=1 */
        {"HR", "<HR SIZE=2>", WC_TAG_TYPE_ONLYOPEN, "", WC_TAG_TYPE_DISABLED, 10},
        /* quotation */
        {"Q", "<blockquote class=\"quote\"><span class=\"inquote\">[q]</span><b>Цитата:</b><br>", 
        WC_TAG_TYPE_1, "<span class=\"inquote\">[/q]</span></blockquote>", WC_TAG_TYPE_1, 10},
        /* Center */
        {"CENTER", "<CENTER>", WC_TAG_TYPE_1, "</CENTER>", WC_TAG_TYPE_1, 10},
        /* Pre */
        {"PRE", "<PRE STYLE=\"margin-left:25px\">", WC_TAG_TYPE_1, "</PRE>", WC_TAG_TYPE_1, 10},
        {"STRIKE", "<STRIKE>", WC_TAG_TYPE_1, "</STRIKE>", WC_TAG_TYPE_1, 10},
        {"SUP", "<SUP>", WC_TAG_TYPE_1, "</SUP>", WC_TAG_TYPE_1, 10},
        {"SUB", "<SUB>", WC_TAG_TYPE_1, "</SUB>", WC_TAG_TYPE_1, 10},
        {"TEX", "<img src=\"http://www.codecogs.com/gif.latex?", WC_TAG_TYPE_1, "\">", WC_TAG_TYPE_1, 10}
};

/* define there smile-like codes
 * and it's translation to HTML
 * first character in tag must be ':' or ';'
 */
SPicConvert PicConvTable[BoardPicCount] = {
        {":))",                "bigsmile.gif",                9},
        {":-))",        "bigsmile.gif",                9},
        {":-)",                "smile.gif",                9},
        {":)",                "smile.gif",                9},
        {":(",                "frown.gif",                9},
        {":-(",                "frown.gif",                9},
        {";)",                "wink.gif",                        9},
        {";-)",                "wink.gif",                        9},
        {":LOL",        "lol.gif",                        9},
        {":!!",                "lol.gif",                        9},
        {":-\\",        "smirk.gif",                9},
        {":\\",                "smirk.gif",                9},
        {":o",                "redface.gif",                9},
        {":MAD",        "mad.gif",                        9},
        {":STOP",        "stop.gif",                        9},
        {":APPL",        "appl.gif",                        9},
        {":BAN",        "ban.gif",                        9},
        {":BEE",        "bee.gif",                        9},
        {":BIS",        "bis.gif",                        9},
        {":ZLOBA",        "blya.gif",                        9},
        {":BORED",        "bored.gif",                9},
        {":BOTAT",        "botat.gif",                9},
        {":COMP",        "comp.gif",                        9},
        {":CRAZY",        "crazy.gif",                9},
        {":DEVIL",        "devil.gif",                9},
        {":DOWN",        "down.gif",                        9},
        {":FIGA",        "figa.gif",                        9},
        {":GIT",        "git.gif",                        9},
        {":GYGY",        "gy.gif",                        9},
        {":HEH",        "heh.gif",                        9},
        {":CIQ",        "iq.gif",                        9},
        {":KURIT",        "kos.gif",                        9},
        {":LAM",        "lam.gif",                        9},
        {":MNC",        "mnc.gif",                        9},
        {":NO",                "no.gif",                        9},
        {":SMOKE",        "smoke.gif",                9},
        {":SORRY",        "sorry.gif",                9},
        {":SUPER",        "super.gif",                9},
        {":UP",                "up.gif",                        9},
        {":YES2",        "yes2.gif",                        9},
        {":YES",        "yes.gif",                        9},
        {":BASH",        "bash.gif",                        9},
        {":CLAPPY",        "clappy.gif",                9},
        {":EWW",        "eww.gif",                        9},
        {":ROTFL",        "roflol.gif",                9},
        {":SPOTMAN","spotman.gif",                9},
        {":WAVE",        "wave.gif",                        9},
        {":COWARD",        "coward.gif",                9},
        {":DRAZNIT","draznit.gif",                9},
        {":ROLLEYES","rolleyes.gif",        9},
        {":PLOHO",        "blevalyanaeto.gif",9},
        {":}",                "icqbig.gif",                9},
};

/* parse string to nearest ';' or ':' */
int inline ParseToOpenSmileTagorHttp(char *s)
{
        register unsigned int i = 0;
#if TRY_AUTO_URL_PREPARSE
        // find smile, ftp or http start
        while(s[i] != 0 && s[i] != ':' && s[i] != ';' && s[i] != 'h' && s[i] != 'f') i++;
#else
        while(s[i] != 0 && s[i] != ':' && s[i] != ';') i++;
#endif
        return i;
}

#if TRY_AUTO_URL_PREPARSE
/* return 1 if url was parsed, 0 otherwise */
static int ReparseUrl(char **ss, char **dd, DWORD status)
{
        int rf = 0;
        char *d = *dd, *s = *ss;
        int slen = strlen(s);
        if( ( (slen > 7) && (strncmp(s, "http://", 7) == 0) ) ||
                ( (slen > 8) && (strncmp(s, "https://", 8) == 0) ) ||
                 ( (slen > 6) && (strncmp(s, "ftp://", 6) == 0) ) )
        {
                if((status & 4)) {
                        //        we have found http:// | https:// | ftp:// reference, so try to parse it
                        char *olds = s;
                        char oldcs;
                        char *dtmp;

                        if(*s == 'h') s++;
                        s+=6;

                        while(*s != 0 && *s != 13 && *s != 10 && *s != ' ' && *s != ')' && *s != '(' 
                                && *s != '>' && *s != '<' && *s != ']' && *s != '[' && *s != ',' 
                                && strncmp(s, "&quot;", 6) !=0 ) s++;
                        oldcs = *s;
                        *s = 0;
                        dtmp = (char*)malloc(strlen(olds)*2+strlen(PARSED_URL_TMPL));
                        sprintf(dtmp, PARSED_URL_TMPL, olds, olds);
                        strcpy(d, dtmp);
                        d+=strlen(dtmp);
                        free(dtmp);
                        *s = oldcs;
                }
                rf = 1;
        }

        *dd = d;
        *ss = s;
        return rf;
}
#endif

/* Smart strcat() function. It removes all duplicated spaces, char #10 and all tabs.
 * The behavior depends of status. If status nonzero it works as usual strcat except
 * #10 char removing.
 * status meaning:
 *                0x01 - spaces and tabs elimination (DISABLING)
 *                0x02 - 13 -> BR parsing
 *                0x04 - url parsing
 */
int inline smartstrcat(char *d, char *s, DWORD status, DWORD *flg)
{
        char *od = d;
        int fstat;
        d += strlen(d);
        *flg = 0;
        if(status & 1) {
                while(*s != 0) {
#if TRY_AUTO_URL_PREPARSE
                        fstat = ReparseUrl(&s, &d, status);
                        if(fstat) *flg = 1;
#endif

                        if(*s != 10) {
                                if(*s == 13 && (status & 2)) {
                                        // convert to <BR>
                                        memcpy(d, "<BR>", 4);
                                        d+=4;
                                }
                                else if(*s == 13 && (status & 0x10)) {
                                                // convert to <BR>
                                                memcpy(d, "&lt;BR&gt;", 10);
                                                d+=10;
                                }
                                else {
                                        *d = *s;
                                        d++;
                                }
                        }
                        s++;
                }
                *d = *s;
        }
        else {
                register int ws = 0; // was space or tab
                while(*s != 0) {
#if TRY_AUTO_URL_PREPARSE
                        fstat = ReparseUrl(&s, &d, status);
                        if(fstat) *flg = 1;
#endif

                        if( *s != 10 && (!(ws && (*s == 0x20 || *s == 0x09))) ) {
                                if(*s == 0x20 || *s == 0x09) {
                                        ws = 1;
                                        *d = 0x20; // space
                                }
                                else {
                                        ws = 0;
                                        if(*s == 13 && (status & 2)) {
                                                // convert to <BR>
                                                memcpy(d, "<BR>", 4);
                                                d+=3;
                                        }
                                        else if(*s == 13 && (status & 0x10)) {
                                                // convert to <BR>
                                                memcpy(d, "&lt;BR&gt;", 10);
                                                d+=9;
                                        }
                                        else *d = *s;
                                }
                                d++;
                        }
                        s++;
                }
                *d = *s;
        }
        return d - od;
}

/* function for concatenation string s to d
 * with converting smile-codes to images
 */
int inline ParseSmiles_smartstrcat(char *d, char *s, BYTE sec, DWORD status, DWORD *flg)
{
        register char *dd = d + strlen(d);
        register char *ss = s;
        unsigned int i = 0, fstat;
        int wassmile = 0;
        *flg = 0;
        while(*ss != 0) {
                i = ParseToOpenSmileTagorHttp(ss);
                if(i) {
                        char si = ss[i];
                        ss[i] = 0;
                        dd += smartstrcat(dd, ss, status, flg);
                        ss[i] = si;
                        ss += i;
                }
                if(*ss == 0) break;

#if TRY_AUTO_URL_PREPARSE
                fstat = ReparseUrl(&ss, &dd, status);
                if(fstat) *flg = 1;
#endif

                for(register unsigned int j = 0; j < BoardPicCount; j++) {
                        if(strlen(PicConvTable[j].tag) <= strlen(ss) && PicConvTable[j].security >= sec &&
                                strncmp(ss, PicConvTable[j].tag, strlen(PicConvTable[j].tag)) == 0 && ((status & 1) == 0) ) {
                                wassmile = 1;
                                strcat(dd, "<IMG BORDER=0 SRC=\"" BOARD_PIC_URL);
                                strcat(dd, PicConvTable[j].url);
                                strcat(dd, "\" ALT=\"");
                                strcat(dd, PicConvTable[j].tag);
                                strcat(dd, "\">");
                                dd += strlen(dd);
                                ss += strlen(PicConvTable[j].tag);
                                goto next_sym;
                        }
                }


                if(*ss == 13 && (status & 2)) {
                        if(*dd == 0) *(dd+4) = 0;
                        // convert to <BR>
                        memcpy(dd, "<BR>", 4);
                        dd+=4;
                }
                else {
                        if(*dd == 0) *(dd+1) = 0;
                        *dd = *ss;
                         dd++;
                }
                ss++;
next_sym: ;
        }
        *dd = 0;
        return wassmile;
}

/* insert string s to string d at position index
 * return d, DOES NOT CONTROL ANY ERRORS
 */
char* strins(char *d, const char *s, int index)
{
        register char *p = d + index; // from
        register char *t = d + index + strlen(s); // to
        register unsigned int x = strlen(d) - index; // count
        for(register int j = x; j >= 0; j--) t[j] = p[j];
        t[x+1] = 0;
        strncpy(p, s, strlen(s));
        return d;
        
}

/* parse string up to nearest =, WC_TAG_OPEN/CLOSE */
int inline ParseRegExp(char *s)
{
        register int i = 0;
        while(s[i] != 0 && s[i] != '=' && s[i] != WC_TAG_CLOSE && s[i] != WC_TAG_OPEN) i++;
        return i;
}

/* parse to nearest WC_TAG_OPEN */
int inline ParseToOpenWC_TAG(char *s)
{
        register int i = 0;
        while(s[i] != 0 && s[i] != WC_TAG_OPEN) i++;
        return i;
}

/* parse to nearest WC_TAG_CLOSE */
int inline ParseToCloseWC_TAG(char *s)
{
        register int i = 0;
        while(s[i] != 0 && s[i] != WC_TAG_CLOSE) i++;
        return i;
}

/* input:        s - parsing string with begining of tag (first symbol = WC_TAG_OPEN)
 * oputput: function return length of parsed board tag or 0 if tag is not valid
 *                                par1 - tag name, par2 - parameter after = in tag (NULL if not present)
 */
int inline ParseBoardTag(char *s, char **par1, char **par2)
{
        char *ss;
        int i, j;
        ss = s;
        *par1 = NULL;
        *par2 = NULL;
        // ignore WC_TAG_OPEN
        if(*s != WC_TAG_OPEN) {
                return 0;
        }
        s++;
        // parse first arg
        i = ParseRegExp(s);
        if(i == 0) {
                return 0;
        }
        *par1=(char*)malloc(i + 1);
        char ts = s[i];
        s[i] = 0;
        strcpy(*par1, s);
        toupperstr(*par1);
        s[i] = ts;
        s += i;
        if(*s == WC_TAG_CLOSE) {
                return s - ss + 1;
        }
        if(*s == '=') {
                // parse second arg
                s++;
                j = ParseToCloseWC_TAG(s);
                if(j == 0) {
                        goto ParseBoardTag_Faild;
                }
                if(s[j] == WC_TAG_CLOSE) {
                        *par2=(char*)malloc(j + 1);
                        char ts = s[j];
                        s[j] = 0;
                        strcpy(*par2, s);
                        s[j] = ts;
                        return s - ss + j + 1;
                }
        }
ParseBoardTag_Faild:
        free(*par1);
        *par1 = NULL;
        *par2 = NULL;
        return 0;
}

/* check and expand board tags -> HTML tags
 * return value: function return 1 if successfull, overwise 0
 * tagtype - tag type (index in TagConvTable) and taglen = 1, 2 
 */
int inline ExpandTag(char *tag1, char *tag2, char **restag, int *tagnumber, int *tagtype, BYTE security)
{
        int tagdirection = 0; /* open by default */
        
        if(tag1[0] == '/')  {
                tagdirection = 1;
                tag1++;
        }
        
        *restag = NULL;
        
        for(int i=0; i < BoardTagCount; i++) {
                if(strcmp(tag1, TagConvTable[i].tag) == 0) {
                        if(TagConvTable[i].security < security) {
                                return 0;
                        }
                        *tagnumber = i;
                        int tagt;
                        if(tagdirection) tagt = TagConvTable[i].typeclose;
                        else tagt = TagConvTable[i].typeopen;
                        switch(tagt) {
                        case WC_TAG_TYPE_1:
                                *tagtype = WC_TAG_TYPE_1;
                                /* check for valid param count */
                                if(tag2 != NULL) return 0;
                                if(tagdirection) {
                                        /* closing */
                                        *restag = (char*)malloc(strlen(TagConvTable[i].tclosetag) + 1);
                                        strcpy(*restag, TagConvTable[i].tclosetag);
                                }
                                else {
                                        /* opening */
                                        *restag = (char*)malloc(strlen(TagConvTable[i].topentag) + 1);
                                        strcpy(*restag, TagConvTable[i].topentag);
                                }
                                return 1;
                                
                        case WC_TAG_TYPE_2:
                                *tagtype = WC_TAG_TYPE_2;
                                /* check for valid param count
                                */
                                if(tag2 == NULL) return 0;
                                if(!tagdirection) {
                                        /* opening */
                                        char *parsedtag2;

                                        if(i == URL_TAG_TYPE) {
                                                if(strncmp(tag2, "http:", 5) != 0 && strncmp(tag2, "ftp:", 4) != 0 &&
                                                    strncmp(tag2, "file:", 5) != 0 && strncmp(tag2, "https:", 6) != 0 &&
                                                        strncmp(tag2, "smb:", 4) != 0)
                                                {
                                                        parsedtag2 = (char*)malloc(strlen(tag2) + 10);
                                                        strcpy(parsedtag2, "http://");
                                                        strcat(parsedtag2, tag2);
                                                }
                                                else parsedtag2 = tag2;
                                        }
                                        else if(i == COLOR_TAG_TYPE) {
                                                if(strncmp(tag2, "#", 1) == 0) tag2++;  
                                                if(strlen(tag2) != 6) return 0;  // 6 symbols should be, format XXXXXX
                                                for(int i=0; i<=5; i++) {        
                                                        if( !( (tag2[i]>= 48 && tag2[i] <= 57) || (tag2[i]>= 65 && tag2[i] <= 70) 
                                                                || (tag2[i]>= 97 && tag2[i] <= 102) )) return 0;  // 0-9,a-f,A-F are allowed
                                                }
                                                parsedtag2 = tag2;
                                        }
                                        else if(i == C_TAG_TYPE) {
                                                if(strncmp(tag2, "#", 1) == 0) tag2++;  
                                                if(strlen(tag2) != 6) return 0;  // 6 symbols should be, format XXXXXX
                                                for(int i=0; i<=5; i++) {        
                                                        if( !( (tag2[i]>= 48 && tag2[i] <= 57) || (tag2[i]>= 65 && tag2[i] <= 70) 
                                                                || (tag2[i]>= 97 && tag2[i] <= 102) )) return 0;  // 0-9,a-f,A-F are allowed
                                                }
                                                parsedtag2 = tag2;
                                        }
                                        else parsedtag2 = tag2;

                                        *restag = (char*)malloc(strlen(TagConvTable[i].topentag) + 1 + strlen(parsedtag2));

                                        /* use sprintf() to insert tag2 into result
                                         * (format of topentag as %s instead of tag2)
                                         */
                                        sprintf(*restag, TagConvTable[i].topentag, parsedtag2);
                                        *restag = (char*)realloc(*restag, strlen(*restag) + 1);
                                        if(parsedtag2 != tag2) free(parsedtag2);
                                }
                                else {
                                        /* closing - not currently supported */
                                        return 0;
                                }
                                return 1;
                                
                        case WC_TAG_TYPE_12:
                                /* temporary not used */
                                
                                return 0;
                                
                        case WC_TAG_TYPE_ONLYOPEN:
                                *tagtype = WC_TAG_TYPE_ONLYOPEN;
                                /* check for valid param count
                                */
                                if(tag2 != NULL) return 0;
                                if(!tagdirection) {
                                        /* opening */
                                        *restag = (char*)malloc(strlen(TagConvTable[i].topentag) + 1);
                                        strcpy(*restag, TagConvTable[i].topentag);
                                }
                                else {
                                        /* closing - not valid (only opening) */
                                        return 0;
                                }
                                return 1;
                        }
                }
        }
        /* tag not found */
        return 0;
}

/* filtering smile codes and board tags using current security level = security
 * and message flags MESSAGE_ENABLED_SMILES MESSAGE_ENABLED_TAGS
 * return value 1 if all successfull, or 0 if string was cutted, 
 * because of ml limitation
 * **** input : s - string, ml - max result string length, Flags - flags of message, security - sec.
 * level of msg (for tags conversion)
 * **** output : r - resulting string

 *        Meaning of some flags
 *                0x80 - set up flag HAVE_URL
 *                0x04 - url autopreparse

 */
int FilterBoardTags(char *s, char **r, BYTE security, DWORD ml, DWORD Flags, DWORD *RF)
{
        //
        int beforePreStatus;
        //
        char *tag1 = NULL, *tag2 = NULL, *res = NULL, *st;
        DWORD opentag, reff, status = 0x04, urldisable = 0;        // http preparse
        int i, StringTooLong = 0;
        SSavedTag OldTag[MAX_NESTED_TAGS];
        
        // ignore starting newline
        while(*s == 10 || *s == 13) s++;
        
        // ignore ending newline
        {
                int sl = strlen(s);
                register int k;
                for(k = sl; k>0; k--) {
                        if(!(s[k - 1] == 10 || s[k - 1] == 13))
                                break;
                }
                s[k] = 0;
        }

        *RF = 0;
        *r = NULL;

        if(Flags & BOARDTAGS_EXPAND_ENTER)
                status |= 0x02;        // 13 -> <BR> conversion

        if(Flags & BOARDTAGS_EXPAND_XMLEN)
                status |= 0x10;        // 13 -> <BR> xml conversion


        if((Flags & BOARDTAGS_CUT_TAGS) || (Flags & BOARDTAGS_TAG_PREPARSE) || ((Flags & BOARDTAGS_PURL_ENABLE) == 0)) {
                status &= (~0x04);        // disable url parsing
                urldisable = 1;
        }

        /* alloc memory for resulting string */
        beforePreStatus = status;
        if(ml < 32000) st = (char *)malloc(32000);
        else st = (char *)malloc(3*ml);

        st[0] = 0;
        opentag = 0;
        for(;;) {
                if(strlen(st) >= ml) {
                        StringTooLong = 1;
                        break;
                }
                if(*s == 0) break;
                i = ParseToOpenWC_TAG(s);
                if(i) {
                        char si = s[i];
                        s[i] = 0;
                        if((Flags & MESSAGE_ENABLED_SMILES) && ((Flags & BOARDTAGS_TAG_PREPARSE) == 0)) {
                                if(ParseSmiles_smartstrcat(st, s, security, status, &reff)) {
                                        *RF = *RF | MESSAGE_ENABLED_SMILES;
                                }
                                if(reff && ((status & 0x80) == 0)) (*RF) |= MESSAGE_HAVE_URL;
                        }
                        else {
                                smartstrcat(st, s, status, &reff);
                                if(reff && ((status & 0x80) == 0)) (*RF) |= MESSAGE_HAVE_URL;
                        }
                        s[i] = si;
                        s += i;
                }
                i = ParseBoardTag(s, &tag1, &tag2);

                if((Flags & MESSAGE_ENABLED_TAGS) == 0)
                        goto ignore_tag;

                if(i) {
                        int tt = 0, tl = 0;

                        /* we have tag, parse  it! */
                        if(!ExpandTag(tag1, tag2, &res, &tt, &tl, security)) {
                                /* invalid tag */
                                goto ignore_tag;
                        }

                        if(tl == WC_TAG_TYPE_ONLYOPEN) {
                                *RF = *RF | MESSAGE_ENABLED_TAGS;
                                if((status & 1) == 0 && ((Flags & BOARDTAGS_TAG_PREPARSE) == 0)) {
                                        // only opened tag - so parse it at once
                                        strcat(st, res);
                                        s += i;
                                }
                                else goto ignore_tag;
                        }
                        else {
                                // if successfully expanded (exists)
                                // already have open tag - so try to interpret it as closing tag
                                if(tag1[0] == '/') {
                                        if(opentag) {
                                                /* closing tag - check for conformity */
                                                if( tl != 1 || OldTag[opentag - 1].tt != tt) {
                                                        /* incompartable tags - ignore it */
                                                        goto ignore_tag;
                                                }

                                                if(tt == PRE_TAG_TYPE || tt == PIC_TAG_TYPE || tt == TEX_TAG_TYPE) // allow tag and space parsing again
                                                        status &= 0xFFFFFFFE;
                                                if(tt == PRE_TAG_TYPE)
                                                        status = beforePreStatus;
                                                /* set flags of message type */
                                                if(tt == PIC_TAG_TYPE || tt == TEX_TAG_TYPE ) { /* there was a picture tag */
                                                        if(!urldisable) {
                                                                status |= 0x04;        // allow http parsing
                                                        }
                                                        status &= (~0x80);
                                                        *RF = *RF | MESSAGE_HAVE_PICTURE;
                                                }
                                                if(tt == URL_TAG_TYPE) { /* there was a url tag */
                                                        if(!urldisable) {
                                                                status |= 0x04;        // allow http parsing
                                                        }
                                                        *RF = *RF | MESSAGE_HAVE_URL;
                                                }
                                                /* have at least one tag */
                                                *RF = *RF | MESSAGE_ENABLED_TAGS;

                                                //        what we will do with tags
                                                if((Flags & BOARDTAGS_TAG_PREPARSE)) {
                                                        // do not change tags, just preparse spaces, etc.
                                                        strins(st, OldTag[opentag - 1].oldexp, OldTag[opentag - 1].index);
                                                        char os = s[i];
                                                        s[i] = 0;
                                                        strcat(st, s);
                                                        s[i] = os;
                                                }
                                                else {
                                                        // expand tags or ignore(cut) them
                                                        if((Flags & BOARDTAGS_CUT_TAGS) == 0) {
                                                                //
                                                                //        PATCH for PIC tag
                                                                //
                                                                if(tt == PIC_TAG_TYPE ) { /* there was a picture tag */
                                                                        char *ts = st + OldTag[opentag - 1].index;
                                                                        if(strncmp(ts, "http:", 5) != 0 && strncmp(ts, "ftp:", 4) != 0 &&
                                                                           strncmp(ts, "file:", 5) != 0 && strncmp(ts, "https:", 6) != 0 &&
                                                                           strncmp(ts, "smb:", 4) != 0)
                                                                        {
                                                                                // add http
                                                                                strins(ts, "http://", 0);
                                                                        }
                                                                }
                                                                strins(st, OldTag[opentag - 1].tagexp, OldTag[opentag - 1].index);
                                                                strins(st, res, strlen(st));
                                                        }
                                                        else {} // tag will be cutted
                                                }
                                                free(OldTag[opentag - 1].tagexp);
                                                free(OldTag[opentag - 1].oldexp);
                                                opentag--; // dec tag count
                                                s += i;
                                        }
                                        else {
                                                /* ignore tags first elem of tag */
                                                goto ignore_tag;
                                        }
                                }
                                else {
                                        if((status & 1) == 0) {
                                                // opening tag
                                                /* tag expanded successfully - set current tag struct */
                                                if(opentag > MAX_NESTED_TAGS - 1) goto ignore_tag;
                                                OldTag[opentag].tt = tt;
                                                OldTag[opentag].tl = tl;
                                                OldTag[opentag].tagexp = res;
                                                res = NULL;
                                                OldTag[opentag].index = strlen(st);
                                                char os = s[i];
                                                s[i] = 0;
                                                OldTag[opentag].oldexp = (char*)malloc(strlen(s) + 1);
                                                strcpy(OldTag[opentag].oldexp, s);
                                                s[i] = os;
                                                s += i;
                                                opentag++; // inc tag count
                                                // check if it was PRE or PIC tag
                                                if(tt == PRE_TAG_TYPE) {
                                                        beforePreStatus = status;
                                                        status &= ~2;//disable "\n" -> "<br>" conv
                                                }
                                                if(tt == PRE_TAG_TYPE || tt == PIC_TAG_TYPE || tt == TEX_TAG_TYPE) // disable tag parsing
                                                        status |= 0x01;
                                                // check if it was PIC tag
                                                if(tt == PIC_TAG_TYPE || tt == TEX_TAG_TYPE) {// disable http parsing
                                                        if(!urldisable)
                                                                status &= (~0x04);
                                                        status |= 0x80;
                                                }
                                                // check if it was URL tag
                                                if(tt == URL_TAG_TYPE) {// disable http parsing
                                                        if(!urldisable)
                                                                status &= (~0x04);
                                                }
                                        }
                                        else goto ignore_tag;
                                }
                        }
                }
                else {
                        /* ignore tags first elem of tag */
                        goto ignore_tag;
                }
                goto parse_next;
ignore_tag:
                if((*s) != 0) {
                        register char si;
                        si = *(s + 1);
                        *(s +1 ) = 0;
                        strcat(st, s);
                        *(s + 1) = si;
                        s++;
                }
parse_next:
                if(tag1) {
                        free(tag1);
                        tag1 = NULL;
                }
                if(tag2) {
                        free(tag2);
                        tag2 = NULL;
                }
                if(res) {
                        free(res);
                        res = NULL;
                }
        }

        if(StringTooLong) {
                /* too long string */
                free(st);
                for(register DWORD i = 0; i<opentag; i++) {
                        free(OldTag[i].oldexp);
                        free(OldTag[i].tagexp);
                }
                return 0;
        }
        if(opentag) {
                register DWORD k;
                for(k = opentag; k > 0; k--)
                        strins(st, OldTag[k - 1].oldexp, OldTag[k - 1].index);

                for(k = 0; k < opentag; k++) {
                        free(OldTag[k].oldexp);
                        free(OldTag[k].tagexp);
                }
        }
        st = (char*)realloc(st, strlen(st) + 5);
        *r = st;
        return 1;
}
