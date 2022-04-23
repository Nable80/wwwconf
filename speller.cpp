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
                        if (fscanf(f,"%[^\n\10]", dic) == EOF && ferror(f))
                                printhtmlerror();

                        if(dic[0] != 0 && (dic[strlen(dic) - 1] == 13 || dic[strlen(dic) - 1] == 10))
                                dic[strlen(dic) - 1] = 0;

                        if (fscanf(f,"%c",&c) == EOF && ferror(f))
                                printhtmlerror();
                        toupperstr(dic);
                        while ((dic[0] != 0 ) && ((sx = strstr(ss, dic)) != NULL)) {
                                x = 1;
                                DWORD tm = (DWORD)strlen(dic);
                                for(DWORD j = 0; j < tm; j++) {
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

#define min(a, b)                       \
({                                      \
        __typeof__(a) aa = (a);         \
        __typeof__(b) bb = (b);         \
        (aa < bb) ? aa : bb;            \
 })

// Detect a unicode symbol in the decimal base NSR format (&#%d;)
// from the begin of string `s' and return its length.
// Return 0 if the symbol isn't detected.
static size_t unicode_len(const char *s)
{
        // "&#%d;" - strlen(s) must be 4 at least
        if (strlen(s) < 4 || *s != '&' || *(s+1) != '#' || !isdigit(*(s+2)))
                return 0;

        const char *ss = s+3;
        while (isdigit(*ss))
                ++ss;
        // 0x10ffff - the highest code point in UCS-2
        if (*ss != ';' || strtoul(s+2, NULL, 10) > 0x10ffff)
                return 0;

        return ss - s + 1;
}

// return filtered string with length not more than ml bytes (include '\0')
char* FilterHTMLTags(const char *s, size_t ml, int allocmem)
{
        char *os, *st;
        static char tb[SPELLER_INTERNAL_BUFFER_SIZE];

        if(allocmem) {
                if (!(st = (char*)malloc(6*strlen(s) + 1))) // 6 == strlen("&quot;")
                        printhtmlerrormes("malloc");
        } else {
                st = tb;
                if (ml > SPELLER_INTERNAL_BUFFER_SIZE)
                        ml = SPELLER_INTERNAL_BUFFER_SIZE;
        }

#define REPLACE(subst)                                               \
{                                                                    \
        if (!(is_enough = st+strlen(subst) <= os+(ml-1)))            \
                break;                                               \
        strcpy(st, subst);                                           \
        ++s;                                                         \
        st += strlen(subst);                                         \
        break;                                                       \
}

        size_t ulen;
        int is_enough = 1;
        os = st;
        while(*s != '\0' && st < os+(ml-1) && is_enough)
                switch (*s) {
                case '<':
                        REPLACE("&lt;");
                case '>':
                        REPLACE("&gt;");
                case '"':
                        REPLACE("&quot;");
                case '&':
                        if ( (ulen = unicode_len(s))) {
                                if (!(is_enough = st+ulen <= os+(ml-1)))
                                        break;
                                strncpy(st, s, ulen);
                                s += ulen;
                                st += ulen;
                                break;
                        } else
                                REPLACE("&amp;");
                default:
                        *st = *s;
                        ++s;
                        ++st;
                }

        *st = '\0';
        if (allocmem)
                if (!(os = (char *)realloc(os, strlen(os) +1)))
                        printhtmlerrormes("realloc");
        return os;
}



/*
 * Controls using of unicode text direction markers.
 * Returns the safety string filtered from `s' that won't change text direction
 * of other page content. The returned string is dynamically allocated, so it
 * must be freed by user.
 *
 * Some technical porridge:
 * There are 4 text direction markers (code points 8234, 8235, 8237, 8238)
 * and a text direction terminator (code point 8236, abbr.: PDF) in Unicode.
 * They affect text direction by stack (push-pop) rule, where a direction
 * marker pushes some direction rule onto a stack and the terminator pops it
 * off.
 * According to standard (http://www.unicode.org/reports/tr20/#Bidi) it is
 * recommended to replace BiDi markers to appropriate tags not to keep them in
 * text.
 * This function filters its input so that there aren't attempts to pop
 * an element from the empty stack and that the stack will be empty at the end
 * of the output string. So the function does two things:
 *  - removes excess PDF characters;
 *  - adds missing PDF characters at the end.
 * We use tags instead of BiDi markers so PDF means a close tag (</span>).
 * Also there is one more thing. Due to existence of weak and neutral 
 * characters it's need to place the special zero-width strong-direction
 * character (Left-to-Right Mark (LRM, &#8206;) or Right-To-Left Mark (RLM,
 * &#8207;)) to guarantee that punctuation and whitespace symbols will
 * have desirable direction. So the function *added* this symbol to the end of the
 * output string (LRM in our case). But:
 * It cuased some errors in displaying on buggy software (i.e. xterm or putty 
 * (buggy) + w3m). In addition LRM placed there always is got to clipboard when
 * text is copypasted.
 * So it was decided to replace LRM by an empty span with the LRE marker.
 *
 * About bidirectional text in Unicode read
 * http://www.iamcal.com/understanding-bidirectional-text/
 *
 * Don't forget that all Unicode symbols are stored in decimal base NCR
 * format.
 */
char *FilterBiDi(const char *s)
{
        size_t level = 0;
        char *os, *ss;

	if (!s)
		return NULL;
        // 7 is a length of NCR for the Unicode BiDi chararacters
        // allocate a max possible size
        if (!(os = (char*) malloc(DESIGN_BIDI_MAXLEN*(strlen(s)/7) + strlen(s)%7 + 1)))
                printhtmlerrormes("malloc");

	if (!(*s)) {
		*os = '\0';
		return os;
	}

        ss = os;

        while (*s != '\0')
                // detect a BiDi symbol
                if (*s == '&'                        &&
                    !strncmp(s+1, "#823", 4)         &&
                    *(s+5) >= '4' && *(s+5) <= '8'   &&
                    *(s+6) == ';') {
                        const char *tag = "";
                        if (*(s+5) == '6') {   // PDF
				// if the stack is empty
				// don't copy unnecessary DESIGN_BIDI_CLOSE
                                if (level) {
                                        tag = DESIGN_BIDI_CLOSE;
                                        --level;
                                }
                        } else {
                                switch (s[5]) {
                                case '4':
                                        tag = DESIGN_BIDI_LRE;
                                        break;
                                case '5':
                                        tag = DESIGN_BIDI_RLE;
                                        break;
                                case '7':
                                        tag = DESIGN_BIDI_LRO;
                                        break;
                                case '8':
                                        tag = DESIGN_BIDI_RLO;
                                        break;
                                }
                                ++level;
                        }
                        strcpy(ss, tag);
                        ss += strlen(tag);
                        s += 7;
                } else {
                        *ss = *s;
                        ++s;
                        ++ss;
                }
        *ss = '\0';

	if (!(os = (char*) realloc(os, strlen(os) + level*strlen(DESIGN_BIDI_CLOSE) + strlen(DESIGN_BIDI_LTR_GUARD) + 1)))
		printhtmlerrormes("realloc");
	
	if (level) {
		size_t i;
		for (i = 0; i < level; ++i)
			strcat(os, DESIGN_BIDI_CLOSE);
	}

        strcat(os, DESIGN_BIDI_LTR_GUARD);

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
        DWORD i = 0, sl = (DWORD)strlen(mask);
        if(sl == 0) return 1;
        while(strlen(IP) > i && sl > i && IP[i] == mask[i]) {
                i++;
        }
        if(sl == i && (mask[i - 1] == '.' || strlen(IP) == i)) return 1;
        return 0;
}

int PrepareTextForPrint(char *msg, char **res, BYTE index, int flags)
{
        DWORD tmp;
        char *st = NULL;
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
                if(FilterBoardTags(st, res, index, MAX_PARAMETERS_STRING, flags, &tmp) == 0)
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
        bool tags_in_header = false;
        bool ignore_tags = false;

        char *st;
        DWORD flg = MESSAGE_ENABLED_TAGS | BOARDTAGS_TAG_PREPARSE;
        if(CFlags & MSG_CHK_DISABLE_WWWCONF_TAGS) {
                flg = flg & (~MESSAGE_ENABLED_TAGS);
        }

        if(FilterBoardTags(mes->MessageHeader, &st, 1, MESSAGE_HEADER_LENGTH, flg, RetFlags) == 0) {
                return MSG_CHK_ERROR_NOMSGHEADER;
        }
        else {
                strcpy(mes->MessageHeader, st);
                free(st);
        }

        if (*RetFlags & MESSAGE_ENABLED_TAGS)
                tags_in_header = true;

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
                        if (fscanf(f, "%254[^ \n]", ip) == EOF && ferror(f))  // get ip
                                printhtmlerror();
                        if(feof(f)) break;
                        if (fscanf(f, "%c", &c) == EOF && ferror(f))  // skip " "
                                printhtmlerror();
                        if(c == '\n') continue;
                        if ((fscanf(f, "%c", &t) == EOF         ||  // type
                             fscanf(f, "%c", &c) == EOF         ||  // skip " "
                             fscanf(f, "%[^\n]", reason) == EOF ||  // ban reason
                             fscanf(f, "%c", &c) == EOF)            // skip \n
                             && ferror(f))
                                printhtmlerror();
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
                st = FilterHTMLTags(mes->MessageHeader, MESSAGE_HEADER_LENGTH);
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
        if(FilterBoardTags(*body, &st, 0, MAX_PARAMETERS_STRING, flg | BOARDTAGS_TAG_PREPARSE, RetFlags) == 0) {
                /* if to long - ignore tags */
                ignore_tags = true;
        }
        else {
                free(*body);
                if(strcmp(st, " ") == 0) *st = 0;
                *body = st;
        }

        if (!ignore_tags && tags_in_header) {
                *RetFlags |= MESSAGE_ENABLED_TAGS;
        }

        return MSG_CHK_ERROR_PASSED;
}

static char* replace(const char *s, const char *find, const char *subst)
{
        const char *sp;
        char *ss, *ssp;
        size_t allocnum;

        if (strlen(subst) > strlen(find))
                allocnum = strlen(subst)*(strlen(s)/strlen(find))
                        + strlen(s)%strlen(subst) + 1;
        else
                allocnum  = strlen(s) + 1;
        
        if ( (ss = (char*) malloc(allocnum)) == NULL)
                printhtmlerror();

        sp = s;
        ssp = ss;

        while ( (sp = strstr(s, find))) {
                strncpy(ssp, s, sp - s);
                ssp += sp - s;
                strcpy(ssp, subst);
                ssp += strlen(subst);
                s = sp + strlen(find);;
        }

        strcpy(ssp, s);
        
        if ( (ss = (char*) realloc(ss, strlen(ss) + 1)) == NULL)
                printhtmlerror();
        
        return ss;
}

char *FilterControl(const char *s)
{
        const char *sp;
        char *ss, *ssp;

        if (!s)
                return NULL;

        if ( (ss = (char*) malloc(strlen(s) + 1)) == NULL)
                printhtmlerror();
        
        sp = s;
        ssp = ss;
        while (*sp) {
                if (*sp != '\n' && *sp != '\t' && *sp != '\r' && *sp > 0 && (*sp < 32 || *sp == 127))
                        *ssp = ' ';
                else
                        *ssp = *sp;
                ++sp;
                ++ssp;
        }
        *ssp = '\0';

        return ss;
}

char *FilterCdata(const char *s)
{
        char *s1, *s2;
        s1 = FilterControl(s);
        s2 = replace(s1, "]]>", "]]]]><![CDATA[>");
        free(s1);
        return s2;
}

// Copy a next sumbol of `src', decode if necessary,
// increase the pointers accordingly.
static void decode_and_copy(char **dst, const char **src)
{
        if ((*src)[0] != '&') {
                *(*dst)++ = *(*src)++;
                return;
        }

        if (strncmp(*src+1, "quot;", 5) == 0) {
                **dst = '"';
                *src += 6;
        } else if (strncmp(*src+1, "gt;", 3) == 0) {
                **dst = '>';
                *src += 4;
        } else if (strncmp(*src+1, "lt;", 3) == 0) {
                **dst = '<';
                *src += 4;
        } else  if (strncmp(*src+1, "amp;", 4) == 0) {
                **dst = '&';
                *src += 5;
        } else {
                **dst = '&';
                ++*src;
        }

        ++*dst;
}

char *DefilterHTMLTags(const char *s)
{
        const char *sp;
        char *ss, *ssp;

        if (!s)
                return NULL;
        
        if ( (ss = (char*) calloc(strlen(s) + 1, 1)) == NULL)
                printhtmlerror();

        sp = s;
        ssp = ss;
        while (*sp)
                decode_and_copy(&ssp, &sp);
        *ssp = '\0';

        if ( (ss = (char*) realloc(ss, strlen(ss) + 1)) == NULL)
                printhtmlerror();

        return ss;
}
