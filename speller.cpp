/***************************************************************************
                          speller.cpp  -  spell, ban, html checker
                             -------------------
    begin                : Mon Mar 19 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "speller.h"
#include "error.h"
#include "messages.h"
#include "boardtags.h"

#define TO_HEX(a) (a > 9 ? (a - 10) + 'A': a + '0')

char* CodeHttpString(char *s, int allocmem, int type)
{
        static char rs[SPELLER_INTERNAL_BUFFER_SIZE];
        char *r, *rr;
        DWORD sl = (DWORD)strlen(s);
        if(allocmem) {
                rr = r = (char*)malloc(3*sl + 10);
        }
        else {
                if(sl > SPELLER_INTERNAL_BUFFER_SIZE/3) return NULL;
                r = (char*)(&rs);
                rr = r;
        }
        for(; *s != 0; s++) {        
                if( ( (*s >= 'A' && *s <='Z') || (*s >= 'a' && *s <='z') || (*s >= '0' && *s <='9') ) || 
                        ( (*s == '.' || *s == '_' || *s == '-' || *s == '~' || *s == '/' || *s == ':') && type == URL_ENCODE ) ||
                        ( (*s == '.' || *s == '_' || *s == '-' || *s == '@') && type == MAIL_ENCODE) ){
                                                                 
                        r[0] = *s;
                        r+=1;
                } else {
                        r[0] = '%';
                        r[1] = TO_HEX( (((*s) & 0x00000000F0) >> 4));
                        r[2] = TO_HEX( ( (*s) & 0x000000000F) );
                        r+=3;
                }
        }
        *r = 0;
        return rr;
}

int IsMailCorrect(char *s)
{
        char *s1;
        if(s == NULL) return 0;
        if((s1 = strchr(s,'@')) == NULL) return 0;
        if(strlen(s1) < 4) return 0;
        if((s = strchr(s1,'.')) == NULL) return 0;
        if(strlen(s) < 2) return 0;
        return 1;
}

int FilterBadWords(char *s)
{
        FILE *f;
        char dic[MAX_STRING];
        char *sx, *ss = (char*)malloc(strlen(s) + 1);
        char c;
        int x = 0;
        strcpy(ss, s);
        ss = toupperstr(ss);
        if((f = fopen(F_BADWORDS, FILE_ACCESS_MODES_R)) != NULL)
        {
                while(!feof(f)) {
                        dic[0] = 0;
                        fscanf(f,"%[^\n\10]", dic);

                        if(dic[0] != 0 && (dic[strlen(dic) - 1] == 13 || dic[strlen(dic) - 1] == 10))
                                dic[strlen(dic) - 1] = 0;

                        fscanf(f,"%c",&c);
                        toupperstr(dic);
                        while ((dic[0] != 0 ) && ((sx = strstr(ss, dic)) != NULL)) {
                                x = 1;
                                register DWORD tm = (DWORD)strlen(dic); 
                                for(register DWORD j = 0; j < tm; j++) {
                                        s[sx - ss + j] = BAD_WORD_SYMBOL;
                                        sx[j] = BAD_WORD_SYMBOL;
                                }
                        }
                }
        }
        else print2log(LOG_WARN_UNABLETOOPENFILE, F_BADWORDS);
        free(ss);
        if(x) return 1;
        else return 0;
}

// return filtered string with length not more than ml bytes (include '\0')
char* FilterHTMLTags(char *s, WORD ml, int allocmem)
{
        char *os, *st;
        static char *tb[SPELLER_INTERNAL_BUFFER_SIZE];
        
        while(*s == '\n') s++;
        
        int h = 0, k = 0;        
        for(k = (int)strlen(s)-1; s[k]=='\n'; k--) h++;
        k = (int)strlen(s);
        s[k - h] = '\0';
        if(allocmem)
                st = (char*)malloc(4*k + 1);
        else {
                st = (char*)(&tb);
        }
        os = st;
        while(*s != '\0' && st - os < ml) {
                if(*s == '<') {
                        strcpy(st, "&lt;");
                        st+= 3;
                }
                else if(*s == '>') {
                        strcpy(st, "&gt;");
                        st+= 3;
                }
                else if(*s == '"') {
                        strcpy(st, "&quot;");
                        st+= 5;
                }
                else if(*s == '&') {
                        strcpy(st, "&amp;");
                        st+= 4;
                }
                else *st = *s;
                s++;
                st++;
        }
        if(st - os > ml) st-=4; // if it was overflow because of '<' or '>'
        *st = '\0';
        if(allocmem) os = (char *)realloc(os, strlen(os) +1);
        return os;
}

// removing all white spaces from beginning and end of string
char* FilterWhitespaces(char *s)
{
        while(isspace(((*s) & 0xff))) s++;
        int h = 0, k = 0;
        for(k = (int)strlen(s)-1; isspace((s[k] & 0xff)); k--) h++;
        s[strlen(s) - h] = '\0';
        return s;
}

// check if IP in mask
// return 1 if successfull, 0 otherwise
int CheckIPinSubnet(char *IP, char *mask)
{
        register DWORD i = 0, sl = (DWORD)strlen(mask);
        if(sl == 0) return 1;
        while(strlen(IP) > i && sl > i && IP[i] == mask[i]) {
                i++;
        }
        if(sl == i && (mask[i - 1] == '.' || strlen(IP) == i)) return 1;
        return 0;
}

void FilterMessageForPreview(char *s, char **dd)
{
        char *d;
        if(!s) {
                *dd = NULL;
                return;
        }
        d = *dd = (char*)malloc(strlen(s) + 1);
        while(*s != 0) {
                if(*s != 10) {
                        *d = *s;
                        d++;
                }
                s++;
        }
        *d = *s;
        *dd = (char*)realloc(*dd, d - (*dd) + 2);
}

int PrepareTextForPrint(char *msg, char **res, BYTE security, int flags)
{
        DWORD tmp;
        char *st;
        int memalloc = 0;
        int spfl = SPELLER_FILTER_HTML | SPELLER_PARSE_TAGS;

        *res = NULL;

        if((SPELLER_FILTER_HTML & spfl) != 0) {
                // if the message to small try to use internal buffers
                if(strlen(msg) < SPELLER_INTERNAL_BUFFER_SIZE/2) {
                        st = FilterHTMLTags(msg, SPELLER_INTERNAL_BUFFER_SIZE - 10, 0);        // do not allocate memory
                }
                else {
                        st = FilterHTMLTags(msg, MAX_PARAMETERS_STRING);
                        memalloc = 1;
                }
        }
        if(!st) return 0;

        if((SPELLER_PARSE_TAGS & spfl) != 0) {
                if(FilterBoardTags(st, res, security, MAX_PARAMETERS_STRING, flags, &tmp) == 0)
                {
                        if(!memalloc) {
                                *res = (char*)malloc(strlen(st) + 1);
                                if(!(*res)) return 0;
                                strcpy(*res, st);
                        }
                        else *res = st;
                }
                else {
                        if(memalloc) free(st);
                }
        }
        else {
                if(!memalloc) {
                        *res = (char*)malloc(strlen(st) + 1);
                        if(!(*res)) return 0;
                        strcpy(*res, st);
                }
                else *res = st;
        }

        return 1;
}

int CheckSpellingBan(struct SMessage *mes, char **body, char **Reason,
                                         DWORD CFlags, DWORD *RetFlags, bool fRegged)
{
        FILE *f;
        char ip[MAX_STRING];
        char *reason = NULL;
        char c, t;

        char *st;
        DWORD flg = 0;
        if(FilterBoardTags(mes->MessageHeader, &st, mes->Security, MESSAGE_HEADER_LENGTH, BOARDTAGS_TAG_PREPARSE, RetFlags) == 0) {
                return MSG_CHK_ERROR_NOMSGHEADER;
        }
        else {
                strcpy(mes->MessageHeader, st);
                free(st);
        }

        if(strlen(mes->MessageHeader) == 0 || strcmp(mes->MessageHeader, " ") == 0) return MSG_CHK_ERROR_NOMSGHEADER;
        
#if MSG_REQURED_BODY
        if(strlen(body) == 0) return MSG_CHK_ERROR_NOMSGBODY;
#endif
        
        // banned check + restriction check
#if BANNED_CHECK
        
        reason = (char*)malloc(MAX_STRING);
        if((f = fopen(F_BANNEDIP, FILE_ACCESS_MODES_R)) != NULL) {
                while(!feof(f)) {
                        bool fProxy = false;
                        ip[0] = 0;
                        fscanf(f, "%254[^ \n]", ip);                // get ip
                        if(feof(f)) break;
                        fscanf(f, "%c", &c);                        // skip " "
                        if(c == '\n') continue;
                        fscanf(f, "%c", &t);                        // type
                        fscanf(f, "%c", &c);                        // skip " "
                        fscanf(f, "%[^\n]", reason);        // ban reason
                        fscanf(f, "%c", &c);                        // skip \n
                        if(CheckIPinSubnet(Cip, ip) &&
                                (t == '*' || (fProxy = ((t|0x20) == 'p' && !fRegged))))
                        {
                                if(fProxy)
                                        strncat(reason, "[PROXY]", MAX_STRING - 1);
                                        *Reason = reason;
                                return MSG_CHK_ERROR_BANNED;
                        }
                        if(CheckIPinSubnet(Cip, ip) && t == '=') {
                            break;
                        }
                }
                fclose(f);
        }
        else print2log(LOG_WARN_UNABLETOOPENFILE, F_BANNEDIP);
        free(reason);
#endif
        
        /* HTML Filtering and WWWConf Tags parsing option */
        
        /* filter HTML in Headers */
        if((CFlags & MSG_CHK_ALLOW_HTML) == 0) {
                // HTML tags check and filtering (%lt, %gt)
                char *st;
                st = FilterHTMLTags(mes->MessageHeader, MESSAGE_HEADER_LENGTH - 1);
                st = FilterWhitespaces(st);
                strcpy(mes->MessageHeader, st);
                free(st);
                st = FilterHTMLTags(mes->AuthorName, AUTHOR_NAME_LENGTH - 1);
                strcpy(mes->AuthorName, st);
                free(st);
        }

        /* filter HTML in Body */
        if((CFlags & MSG_CHK_ALLOW_HTML) == 0) {
                if(*body != NULL) {
                        char *st = FilterHTMLTags(*body, MAX_PARAMETERS_STRING - 1);
                        free(*body);
                        *body = st;
                }
        }

#if SPELLING_CHECK == 1
        // spelling check
        // if disallowed word was found in header we should boiling out
        if(FilterBadWords(mes->MessageHeader)) return MSG_CHK_ERROR_BADSPELLING;
        if(FilterBadWords(mes->AuthorName)) return MSG_CHK_ERROR_BADSPELLING;
        // we should only replace with '#' all bad words in body
        FilterBadWords(*body);
#endif

        /* parse WWWConf Tags and smiles */

        /* set flags for body */
        flg = MESSAGE_ENABLED_SMILES | MESSAGE_ENABLED_TAGS;

        if(CFlags & MSG_CHK_DISABLE_SMILE_CODES) {
                flg = flg & (~MESSAGE_ENABLED_SMILES);
        }
        if(CFlags & MSG_CHK_DISABLE_WWWCONF_TAGS) {
                flg = flg & (~MESSAGE_ENABLED_TAGS);
        }

        /* parse body */
        if(FilterBoardTags(*body, &st, mes->Security, MAX_PARAMETERS_STRING, flg | BOARDTAGS_TAG_PREPARSE, RetFlags) == 0) {
                /* if to long - ignore tags */
                *RetFlags = 0;
        }
        else {
                free(*body);
                if(strcmp(st, " ") == 0) *st = 0;
                *body = st;
        }

        return MSG_CHK_ERROR_PASSED;
}
