/***************************************************************************
                             dbase.cpp  -  database message engine
                                -------------------
        begin                : Wed Mar 14 2001
        copyright            : (C) 2001 by Alexander Bilichenko
        email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "dbase.h"
#include "error.h"
#include "security.h"
#include "speller.h"
#include "messages.h"
#include "boardtags.h"
#include "profiles.h"
#include "freedb.h"
#include "sendmail.h"
#include "main.h"
#include "messages.h"
#include "dbaseutils.h"

static int curcolor = 1;


// current message displaying parameters
DWORD currentlsel;
DWORD currenttc;
DWORD currenttt;
DWORD currenttv;
DWORD currentss;
DWORD currentdsm;
DWORD currenttopics;
DWORD currentlm;
DWORD currentfm;
DWORD currentlt;
DWORD currentft;
DWORD currentlann;
DWORD topicsoverride;
int currenttz;

DWORD cookie_lsel;
DWORD cookie_tc;
DWORD cookie_tt;
DWORD cookie_tv;
DWORD cookie_ss;
DWORD cookie_dsm;
DWORD cookie_topics;
long cookie_tz;

char *cookie_seq;
char *cookie_name;

time_t current_minprntime;

CUserLogin ULogin;

#if USER_ALT_NICK_SPELLING_SUPPORT
static bool cninit;
CAltNamesParser AltNames(F_PROF_ALTNICK, cninit);
#endif

int HPrinted = 0;

char *Cip;
DWORD Nip = 0;
#if ALLOW_MARK_NEW_MESSAGES == 2
DWORD newhref = 0;
#endif

char DESIGN_open_dl[10];
char DESIGN_open_dl_grey[20];
char DESIGN_open_dl_white[20];
char DESIGN_close_dl[10];
char DESIGN_threads_divider[500];
char DESIGN_break[10];

int IP2HostName(DWORD IP, char *hostname, int maxlen)
{
        struct hostent *he;
#if IP_TO_HOSTNAME_RESOLVE
        if((he = gethostbyaddr((char*)(&IP), 4, AF_INET)) != NULL) {
                if(strlen(he->h_name) > 0)
                        strncpy(hostname, he->h_name, maxlen);
                else
                        return 0;
        } else
                return 0;
#else
        strcpy(hostname, inet_ntoa(IP), maxlen);
#endif
        hostname[maxlen - 1] = 0;
        return 1;
}

char* ConvertTime(time_t t)
{
        static char s[100];
        tm *tt;

        t += 3600*currenttz;
        tt = gmtime(&t);
        sprintf(s, "%d/%d/%d %d:%02d", tt->tm_mday, tt->tm_mon + 1, tt->tm_year + 1900, tt->tm_hour, tt->tm_min);
        return s;
}

char* ConvertFullTime(time_t t)
{
        const char *days[7] = {
                MESSAGEMAIN_DATETIME_DAY_SUN,
                MESSAGEMAIN_DATETIME_DAY_MON,
                MESSAGEMAIN_DATETIME_DAY_TEU,
                MESSAGEMAIN_DATETIME_DAY_WED,
                MESSAGEMAIN_DATETIME_DAY_THU,
                MESSAGEMAIN_DATETIME_DAY_FRI,
                MESSAGEMAIN_DATETIME_DAY_SAT
        };
        const char *months[12] = {
                MESSAGEMAIN_DATETIME_JAN,
                MESSAGEMAIN_DATETIME_FEB,
                MESSAGEMAIN_DATETIME_MAR,
                MESSAGEMAIN_DATETIME_APR,
                MESSAGEMAIN_DATETIME_MAY,
                MESSAGEMAIN_DATETIME_JUN,
                MESSAGEMAIN_DATETIME_JUL,
                MESSAGEMAIN_DATETIME_AUG,
                MESSAGEMAIN_DATETIME_SEP,
                MESSAGEMAIN_DATETIME_OCT,
                MESSAGEMAIN_DATETIME_NOV,
                MESSAGEMAIN_DATETIME_DEC
        };
        static char s[100];
        tm *tt;

        t += 3600*currenttz;
        tt = gmtime(&t);
        sprintf(s, "%s, %s %d %d:%02d:%02d %d", days[tt->tm_wday], months[tt->tm_mon], tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec, tt->tm_year + 1900);
        return s;
}

void DB_Base::Profile_UserName(char *name, char *tostr, int reg, int doparsehtml)
{
        char *str, *str1;
        if(reg) {
                char *name_dec;
                if(doparsehtml) str = FilterHTMLTags(name, AUTHOR_NAME_LENGTH*3+1, 0);
                else str = name;
                str = FilterBiDi(str);
                name_dec = DefilterHTMLTags(name);
                str1 = CodeHttpString(name_dec, 0);        // do not allocate memory, use internal buffer
                free(name_dec);
                sprintf(tostr, DESIGN_MESSAGE_REG, str1, str);
        } else {
		str = FilterBiDi(name);
                sprintf(tostr, DESIGN_MESSAGE_UNREG, str);
	}
        free(str);
}

/* this function could not print more than 32000 messages at once */
int DB_Base::printThreadbuffer(SMessage *buf, DWORD size, int p, DWORD fmpos, int ll,
                                                          int *czero, DWORD selected, DWORD root, int *bp)
{
        if(p) {
                DWORD count = size/sizeof(SMessage);
                for(DWORD i = 0; i < count; i++) {
                        
                        if(fmpos + i*sizeof(SMessage) == root) (*bp) = 1;
                        if((*bp)){
                                if(invflag != -1 || ((buf[i].Flag & MESSAGE_IS_INVISIBLE) != 0))
                                {
                                        if(((buf[i].Flag & MESSAGE_IS_INVISIBLE) != 0) && (invflag == -1))
                                        {
                                                invflag = buf[i].Level;
                                        }
                                        else
                                        {        // (buf[i].Flag & MESSAGE_IS_INVISIBLE) == TRUE, invflag != -1
                                                // seems that invisibility was already started
                                                if(invflag >= buf[i].Level)
                                                {
                                                        if((buf[i].Flag & MESSAGE_IS_INVISIBLE) == 0)
                                                        {
                                                                // invisible message ended
                                                                invflag = -1;
                                                                goto L_BVisible1;
                                                        }
                                                        else
                                                        {
                                                                // next message invisible too
                                                                invflag = buf[i].Level;
                                                        }
                                                }
                                        }
                                        // if admin, show all
                                        if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) goto L_BVisible1;
                                }
                                else {
L_BVisible1:
                                        // we are printing content of a thread, READING THREAD
                                        if(buf[i].Level != ll)
                                        {
                                                if(ll > buf[i].Level)
                                                {
                                                        int x = ll - buf[i].Level;
                                                        for(int j = 0; j < x ; j++) printf("%s", DESIGN_close_dl);
                                                        ll = buf[i].Level;
                                                }
                                                else
                                                {
                                                        // only 1 level increase allowed
                                                        printf("%s", DESIGN_open_dl);
                                                        ll = buf[i].Level;
                                                }
                                        }
                                        else if (buf[i].Level > 0) {
                                                // br before messages on one level, READING, 111!!
                                                printf("%s", DESIGN_break);
                                        }

                                        if(buf[i].Level == 0)
                                        {
                                                (*czero)++;
                                                if(*czero == 2)
                                                {
                                                        return ll;
                                                }
                                        }
                                
                                        if(selected != buf[i].ViIndex)
                                        {
                                                printhtmlmessage_in_index(&buf[i], MESSAGE_INDEX_PRINT_ITS_URL | MESSAGE_INDEX_DISABLE_ROLLED);
                                        }
                                        else
                                        {
                                                printhtmlmessage_in_index(&buf[i], MESSAGE_INDEX_DISABLE_ROLLED);
                                        }
                                }
                        }
                }
        }
        else {
                DWORD count = size/sizeof(SMessage) - 1;
                for(signed long i = count; i >= 0; i--)
                {
                        
                        if(fmpos + i*sizeof(SMessage) == root) (*bp) = 1;
                        if(*bp)
                        {
                                if(invflag != -1 || ((buf[i].Flag & MESSAGE_IS_INVISIBLE) != 0))
                                {
                                        if(((buf[i].Flag & MESSAGE_IS_INVISIBLE) != 0) && (invflag == -1))
                                        {
                                                invflag = buf[i].Level;
                                        }
                                        else
                                        {
                                                if(invflag >= buf[i].Level)
                                                {
                                                        if((buf[i].Flag & MESSAGE_IS_INVISIBLE) == 0)
                                                        {
                                                                // invisible message ended
                                                                invflag = -1;
                                                                goto L_BVisible2;
                                                        }
                                                        else
                                                        {
                                                                // next message invisible too
                                                                invflag = buf[i].Level;
                                                        }
                                                }
                                        }
                                        // if superuser, show all
                                        if(ULogin.LU.right & USERRIGHT_SUPERUSER) goto L_BVisible2;
                                }
                                else
                                {
L_BVisible2:
                                        if(buf[i].Level != ll)
                                        {
                                                if(ll > buf[i].Level)
                                                {
                                                        int x = ll - buf[i].Level;
                                                        for(int j = 0; j < x ; j++) printf("%s", DESIGN_close_dl);
                                                        ll = buf[i].Level;
                                                }
                                                else
                                                {
                                                        // only 1 level increase allowed
                                                        printf("%s", DESIGN_open_dl);
                                                        ll = buf[i].Level;
                                                }
                                        }
                                        else if (buf[i].Level > 0) {
                                                // br before messages on one level, READING, 111!!
                                                printf("%s", DESIGN_break);
                                        }

                                        if(buf[i].Level == 0)
                                        {
                                                (*czero)++;
                                                if(*czero == 2)
                                                {
                                                        return ll;
                                                }
                                        }
                                        if(selected != buf[i].ViIndex)
                                        {
                                                printhtmlmessage_in_index(&buf[i], MESSAGE_INDEX_PRINT_ITS_URL | MESSAGE_INDEX_DISABLE_ROLLED);
                                        }
                                        else
                                        {
                                                printhtmlmessage_in_index(&buf[i], MESSAGE_INDEX_DISABLE_ROLLED);
                                        }
                                }
                        }
                }
        }
        return ll;
}

/* this function could not print more than 32000 messages at once */
int DB_Base::printhtmlbuffer(SMessage *buf, DWORD size, int p/*direction*/, int *ll, int *pr, DWORD mode, DWORD &shouldprint, DWORD &skipped)
{
        
        if(p)
        {
                DWORD count = size/sizeof(SMessage);
                for(DWORD i = 0; i < count; i++) {


                        if(buf[i].Level == 0) {
                                
                                invflag = -1;
                                collapsed = 0;

                                // check should we stop printing because of data limitation
                                if(buf[i].Date < current_minprntime && currentlsel == 1) {
                                        return 0;
                                }

#if TOPICS_SYSTEM_SUPPORT
                                // check for topic match our topic mask
                                if( !((1 << buf[i].Topics) & currenttopics) ){
                                        invflag = 0;
                                        collapsed = 1;
                                        skipped |= 0xf0000000;
                                        continue;
                                }
#endif

                        }

                        // check for view mode == all rolled
                        if( ((mode & PRINTMODE_ALL_ROLLED) || (mode & PRINTMODE_XML)) && buf[i].Level == 0) {
                                invflag = buf[i].Level;
                                collapsed = 1;
                                /* show first message of the thread */
                                if(!(buf[i].Flag & MESSAGE_IS_INVISIBLE)) goto L_BVisible1;
                        }

                        if(invflag != -1 || (buf[i].Flag & MESSAGE_IS_INVISIBLE) ||
                                (buf[i].Flag & MESSAGE_COLLAPSED_THREAD))
                        {
                                
                                if((invflag == -1) && ((buf[i].Flag & MESSAGE_IS_INVISIBLE) ||
                                        (buf[i].Flag & MESSAGE_COLLAPSED_THREAD)))
                                {
                                        //
                                        //        Starting invsible/rolled message check
                                        //
                                        invflag = buf[i].Level;
                                        if(buf[i].Flag & MESSAGE_COLLAPSED_THREAD)
                                        {
                                                collapsed = 1;
                                                // can we show it ?
                                                if((buf[i].Flag & MESSAGE_IS_INVISIBLE) != 0) {
                                                        if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) {
                                                                /* show first message of the rolled thread */
                                                                goto L_BVisible1;
                                                        }
                                                }
                                                else goto L_BVisible1;
                                        }
                                }
                                else
                                {
                                        //
                                        //        Check for stopping rolled/invisible thread
                                        //
                                        if(invflag >= buf[i].Level)
                                        {
                                                if(buf[i].Flag & MESSAGE_IS_INVISIBLE)
                                                {
                                                        /* next message invisible too */
                                                        invflag = buf[i].Level;

                                                        if((ULogin.LU.right & USERRIGHT_SUPERUSER)) {
                                                                if((buf[i].Flag & MESSAGE_COLLAPSED_THREAD) != 0) {
                                                                        collapsed = 1;
                                                                        goto L_BVisible1;
                                                                }
                                                                else collapsed = 0;
                                                        }
                                                }
                                                else
                                                {
                                                        /* invisible or collapsed message ended or not ?*/
                                                        if((buf[i].Flag & MESSAGE_COLLAPSED_THREAD) == 0 && collapsed) {
                                                                /* next message collapsed too */
                                                                invflag = -1;
                                                                collapsed = 0;
                                                        }
                                                        else if(collapsed == 0 && (buf[i].Flag & MESSAGE_COLLAPSED_THREAD) != 0) {
                                                                collapsed = 1;
                                                        }
                                                        else if(collapsed == 1 && (buf[i].Flag & MESSAGE_COLLAPSED_THREAD) != 0) {
                                                        }
                                                        else {
                                                            invflag = -1;
                                                        }
                                                        goto L_BVisible1;
                                                }
                                        }
                                }

                                /* if admin, show all invisible messages */
                                if((ULogin.LU.right & USERRIGHT_SUPERUSER)  && (!collapsed))
                                        goto L_BVisible1;
#if ALLOW_MARK_NEW_MESSAGES
                                /* check for new message mark */
                                if((collapsed) && currentlm < buf[i].ViIndex && (skipped & 0xf0000000) == 0) {
                                        if(newmessflag) {
                                                if(lastmes.Date < buf[i].Date)
                                                        memcpy(&lastmes, &(buf[i]), sizeof(SMessage));
                                        }
                                        else memcpy(&lastmes, &(buf[i]), sizeof(SMessage));
                                         newmessflag++;
                                }
#endif
                                if(buf[i].Level == 0) skipped |= 0xf0000000;
                                if((skipped & 0xf0000000) == 0) skipped++;
                        }
                        else {
L_BVisible1:
                                /* check if it's end of thread and if it is increase already printed count
                                * also check should we stop printing
                                */
                                
                                if(currentlm < buf[i].ViIndex){
                                        nm_counter ++;
                                        if(buf[i].Level == 0)
                                                nt_counter ++; 
                                }
                                
                                if(buf[i].Level == 0)
                                {
                                        alrprn++;
                                        // check should we stop printing
                                        if( (currentlsel == 1 && buf[i].Date < current_minprntime) ||
                                                (currentlsel == 2 && alrprn == currenttc + 1) )
                                        {
                                                return 0;
                                        }
                                }

                                if(shouldprint != 0xFFFFFFFF) {
                                
                                        skipped &= (~0xf0000000);
                                        // delayed print done
                                        if((mode & PRINTMODE_XML) == 0) {
                                                printhtmlmessage_in_index(&pmes, MESSAGE_INDEX_PRINT_ITS_URL, skipped, newmessflag);
                                                printf("<div id=d%ld style=\"display: none;\">", pmes.ViIndex);
                                        }
                                        else printxmlmessage_in_index(&pmes);
                                        
                                        newmessflag = 0;
                                        shouldprint = 0xFFFFFFFF;
                                        skipped = 0;
                                }

                                if((mode & PRINTMODE_XML) == 0) {
                                        // there more than 1 message in thread, MAIN INDEX!
                                        if(buf[i].Level != (*ll)) {
                                                if((*ll) > buf[i].Level) {
                                                        int x = (*ll) - buf[i].Level;
                                                        for(int j = 0; j < x + 1 ; j++) printf("</div>");
                                                }
                                                (*ll) = buf[i].Level;
                                        }
                                        else {
                                                printf("</div>");
                                        }
                                

                                        if(buf[i].Level == 0 && (*pr)) {
                                                printf("%s%s",DESIGN_close_dl, DESIGN_threads_divider);
                                                if(curcolor) printf("%s", DESIGN_open_dl_white);
                                                else printf("%s", DESIGN_open_dl_grey);
                                                curcolor = !curcolor;
                                        }
                                        else (*pr) = 1;
                                }

                                if(!collapsed) {
                                        if((mode & PRINTMODE_XML) == 0) {
                                                printhtmlmessage_in_index(&buf[i], MESSAGE_INDEX_PRINT_ITS_URL);
                                                printf("<div id=d%ld>", buf[i].ViIndex);
                                        }
                                        else printxmlmessage_in_index(&buf[i]);
                                        
                                }
                                else {
                                        shouldprint = i;
                                        skipped = 0;
                                        newmessflag = 0;
                                        memcpy(&pmes, &buf[i], sizeof(SMessage));
                                }
                        }
                }
        }
        else {
                DWORD count = size/sizeof(SMessage) - 1;
                for(signed long i = count; i >= 0; i--)
                {

                        if(buf[i].Level == 0) {
                                
                                collapsed = 0;
                                invflag = -1;

                                // check should we stop printing because of data limitation
                                if(buf[i].Date < current_minprntime && currentlsel == 1) {
                                        return 0;
                                }

#if TOPICS_SYSTEM_SUPPORT
                                // check for topic match our topic mask
                                if( !((1 << buf[i].Topics) & currenttopics) )
                                {
                                        invflag = 0;
                                        collapsed = 1;
                                        skipped |= 0xf0000000;
                                        continue;
                                }
#endif

                        }

                        // check for view mode == all rolled
                        if( ((mode & PRINTMODE_ALL_ROLLED) || (mode & PRINTMODE_XML)) && buf[i].Level == 0) {
                                invflag = buf[i].Level;
                                collapsed = 1;
                                /* show first message of the thread */
                                if(! (buf[i].Flag & MESSAGE_IS_INVISIBLE)) goto L_BVisible2;
                        }

                        if(invflag != -1 || (buf[i].Flag & MESSAGE_IS_INVISIBLE) ||
                                (buf[i].Flag & MESSAGE_COLLAPSED_THREAD))
                        {
                                
                                if((invflag == -1) && ((buf[i].Flag & MESSAGE_IS_INVISIBLE) ||
                                        (buf[i].Flag & MESSAGE_COLLAPSED_THREAD)))
                                {
                                        //
                                        //        Starting invsible/rolled message check
                                        //
                                        invflag = buf[i].Level;
                                        if(buf[i].Flag & MESSAGE_COLLAPSED_THREAD)
                                        {
                                                collapsed = 1;
                                                // can we show it ?
                                                if((buf[i].Flag & MESSAGE_IS_INVISIBLE) != 0) {
                                                        if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) {
                                                                /* show first message of the rolled thread */
                                                                goto L_BVisible2;
                                                        }
                                                }
                                                else goto L_BVisible2;
                                        }
                                }
                                else
                                {
                                        //
                                        //        Check for stopping rolled/invsible thread
                                        //
                                        if(invflag >= buf[i].Level)
                                        {
                                                if(buf[i].Flag & MESSAGE_IS_INVISIBLE)
                                                {
                                                        /* next message invisible too */
                                                        invflag = buf[i].Level;

                                                        if((ULogin.LU.right & USERRIGHT_SUPERUSER)) {
                                                                if((buf[i].Flag & MESSAGE_COLLAPSED_THREAD) != 0) {
                                                                        collapsed = 1;
                                                                        goto L_BVisible2;
                                                                }
                                                                else collapsed = 0;
                                                        }
                                                }
                                                else
                                                {
                                                        /* invisible or collapsed message ended or not ?*/
                                                        if((buf[i].Flag & MESSAGE_COLLAPSED_THREAD) == 0 && collapsed) {
                                                                /* next message collapsed too */
                                                                invflag = -1;
                                                                collapsed = 0;
                                                        }
                                                        else if(collapsed == 0 && (buf[i].Flag & MESSAGE_COLLAPSED_THREAD) != 0) {
                                                                collapsed = 1;
                                                        }
                                                        else if(collapsed == 1 && (buf[i].Flag & MESSAGE_COLLAPSED_THREAD) != 0) {
                                                        }
                                                        else {
                                                            invflag = -1;
                                                        }
                                                        goto L_BVisible2;
                                                }
                                        }
                                }
                                /* if admin, show all invisible messages */
                                if((ULogin.LU.right & USERRIGHT_SUPERUSER) && (!collapsed))
                                        goto L_BVisible2;
#if ALLOW_MARK_NEW_MESSAGES
                                /* check for new message mark */
                                if((collapsed) && currentlm < buf[i].ViIndex && (skipped & 0xf0000000) == 0) {
                                        if(newmessflag) {
                                                if(lastmes.Date < buf[i].Date)
                                                        memcpy(&lastmes, &(buf[i]), sizeof(SMessage));
                                        }
                                        else memcpy(&lastmes, &(buf[i]), sizeof(SMessage));
                                        newmessflag++;
                                }
#endif
                                if(buf[i].Level == 0) skipped |= 0xf0000000;
                                if((skipped & 0xf0000000) == 0) skipped++;
                        }
                        else
                        {
L_BVisible2:
                                /* check if it's end of thread and if it is increase already printed count
                                * also check should we stop printing 
                                */
                                
                                if(currentlm < buf[i].ViIndex){
                                        nm_counter ++;
                                        if(buf[i].Level == 0)
                                                nt_counter ++; 
                                }
                                
                                if(buf[i].Level == 0)
                                {
                                        alrprn++;
                                        // check should we stop printing
                                        if( (currentlsel == 1 && buf[i].Date < current_minprntime) ||
                                                (currentlsel == 2 && alrprn == currenttc + 1) )
                                        {
                                                return 0;
                                        }
                                }

                                if(shouldprint != 0xFFFFFFFF) {

                                        skipped &= (~0xf0000000);
                                        // delayed print done

                                        if((mode & PRINTMODE_XML) == 0) {
                                                printhtmlmessage_in_index(&pmes, MESSAGE_INDEX_PRINT_ITS_URL, skipped, newmessflag);
                                                printf("<div id=d%ld style=\"display: none;\">", pmes.ViIndex);
                                        }
                                        else printxmlmessage_in_index(&pmes);
                                        
                                        newmessflag = 0;
                                        shouldprint = 0xFFFFFFFF;
                                        skipped = 0;
                                }
                                
                                if((mode & PRINTMODE_XML) == 0) {

                                        // only one message in (sub) thread, could be collapsed, MAIN INDEX
                                        if(buf[i].Level != (*ll)) {
                                                if((*ll) > buf[i].Level) {
                                                        int x = (*ll) - buf[i].Level;
                                                        for(int j = 0; j < x + 1; j++) printf("</div>");
                                                }
                                                (*ll) = buf[i].Level;
                                        }
                                        else {
                                                printf("</div>");
                                        }

                                        if(buf[i].Level == 0 && (*pr)) {
                                                printf("%s%s", DESIGN_close_dl, DESIGN_threads_divider);
                                                if(curcolor) printf("%s", DESIGN_open_dl_white);
                                                else printf("%s", DESIGN_open_dl_grey);
                                                curcolor = !curcolor;
                                        }
                                        else (*pr) = 1;
                                }

                                if(!collapsed) {
                                        if((mode & PRINTMODE_XML) == 0) {
                                                printhtmlmessage_in_index(&buf[i], MESSAGE_INDEX_PRINT_ITS_URL);
                                                printf("<div id=d%ld>", buf[i].ViIndex);
                                        }
                                        else printxmlmessage_in_index(&buf[i]);
                                        
                                }
                                else {
                                        shouldprint = i;
                                        skipped = 0;
                                        newmessflag = 0;
                                        memcpy(&pmes, &buf[i], sizeof(SMessage));
                                }

                        }
                }
        }
        return 1;
}

int DB_Base::DB_PrintHtmlIndex(DWORD mtc)
{
        curcolor = (mtc % 2);

#if TOPICS_SYSTEM_SUPPORT
        // if we have NULL topics - stop printing
        if(currenttopics == 0)
                return 0;
#endif

        switch(currentss) {
        case 1:
                return printhtmlindexhron_bythreads(PRINTMODE_NULL);
        case 2:
                return printhtmlindexhron_bythreads(PRINTMODE_NULL);
        case 3:
                return printhtmlindexhron_wothreads();
        case 4:
                return printhtmlindexhron_bythreads(PRINTMODE_ALL_ROLLED);
        case 5:
                return printhtmlindexhron_bythreads(PRINTMODE_XML);
        }
        return 1;
}

int DB_Base::printhtmlmessage_in_index(SMessage *mes, int style, DWORD skipped, int newmessmark)
{

        // *******************************
        // BUG BUG with aname
        //////////////////////////////////
        char *mp = NULL, aname[1000], *tm;
        DWORD ff;
        
        printf("<span id=m%lu>", mes->ViIndex);

#if ALLOW_MARK_NEW_MESSAGES == 2
        if((currentdsm & CONFIGURE_plu) != 0) {
                if(currentlm < mes->ViIndex || newmessmark) {
                  printf(TAG_NEW_MSG_MARK_HREF, mes->ViIndex, newhref, newhref+1);
                  ++newhref;
                }
        } else
                if(currentlm < mes->ViIndex || newmessmark) printf(TAG_NEW_MSG_MARK);
#endif
#if ALLOW_MARK_NEW_MESSAGES == 1
        if(currentlm < mes->ViIndex || newmessmark) printf(TAG_NEW_MSG_MARK);
#endif

        tm = ConvertTime(mes->Date);

        if(!FilterBoardTags(mes->MessageHeader, &mp, 1, MAX_PARAMETERS_STRING, mes->Flag, &ff))
                mp = mes->MessageHeader;

        // does this message posted by registred user ?
        if (mes->UniqUserID == 0) {
                char *tmp = FilterBiDi(mes->AuthorName);
                sprintf(aname, DESIGN_UNREGISTRED_NICK, tmp);
                if (tmp)
                        free(tmp);
        } else {
                char altnick[1000];
                char *altnick_f;

                if((currentdsm & CONFIGURE_nalt) == 0 && AltNames.NameToAltName(mes->UniqUserID, altnick)) {
                        char *st;
                        if(PrepareTextForPrint(altnick, &st, 1, MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE))
                                strcpy(altnick, st);
                } else
                        strcpy(altnick, mes->AuthorName);

                altnick_f = FilterBiDi(altnick);

                // if this, is does this user view this message ;-) ?
                if( ((currentdsm & CONFIGURE_onh) == 0) && ULogin.LU.ID[0] !=0 && mes->UniqUserID == ULogin.LU.UniqID ) {
                        sprintf(aname, DESIGN_REGISTRED_OWN_NICK, altnick_f);
                }
                else {
                        char *ts;
                        // maybe user has selected some nicks to be displayed as detailed nicks?
                        if( (ULogin.LU.ID[0] != 0 && (ts = strstr(ULogin.pfui->SelectedUsers, mes->AuthorName)) != NULL) &&
                                (ts[strlen(mes->AuthorName)] == '\n' || ts[strlen(mes->AuthorName)] == '\r' || ts[strlen(mes->AuthorName)] == '\0') && 
                                (ts == ULogin.pfui->SelectedUsers || ts[-1] == '\n' || ts[-1] == '\r'  ) ){ 
//                        if(ULogin.LU.ID[0] != 0 && ((ts = strstr(ULogin.pfui->SelectedUsers, mes->AuthorName)) != NULL)) {
                                sprintf(aname, DESIGN_SELECTEDUSER_NICK, altnick_f);
                        }
                        else {
                                sprintf(aname, DESIGN_REGISTRED_NICK, altnick_f);
                        }
                }
                if (altnick_f)
                        free(altnick_f);
        }


        if(mes->Flag & MESSAGE_IS_CLOSED) printf(TAG_MSG_CLOSED_THREAD, mes->ViIndex);

        if((mes->Flag & MESSAGE_IS_INVISIBLE) != 0) 
                printf("<strike>");

        printf("<A NAME=%ld", mes->ViIndex);
        if(MESSAGE_INDEX_PRINT_ITS_URL & style)
                printf(" HREF=\"%s?read=%lu\"",MY_CGI_URL, mes->ViIndex);
        if(MESSAGE_INDEX_PRINT_BLANK_URL & style)
                printf(" TARGET=\"_blank\"");
        if((MESSAGE_INDEX_PRINT_ITS_URL & style) == 0)
                printf("><B>");
        else
                printf(">");
#if        TOPICS_SYSTEM_SUPPORT
        if(mes->Topics <= TOPICS_COUNT - 1 && mes->Level == 0 && mes->Topics  != 0) {
                printf(DESIGN_TOPIC_TAG_OPEN "%s" DESIGN_TOPIC_TAG_CLOSE DESIGN_TOPIC_DIVIDER,Topics_List[mes->Topics]);
        }
#endif
        // subject
        char *aheader = FilterBiDi(mp);
        printf(DESING_INDEX_MSG_HEADER, aheader);
        if (aheader)
                free(aheader);
        if ((MESSAGE_INDEX_PRINT_ITS_URL & style) == 0)
                printf("</B>");
        printf("</A>");

        printf(DESIGN_WRAP DESIGN_NOWRAP_START);
        if (MESSAGE_INDEX_PRINT_ITS_URL & style)
                printf("<a href=\"%s?read=%lu\">", MY_CGI_URL, mes->ViIndex);
        printf("<span class=\"marker\">");
        if (mes->Flag & MESSAGE_HAVE_URL)
                printf("(" TAG_MSG_HAVE_URL ")");
        if (mes->Flag & MESSAGE_HAVE_PICTURE)
                printf("(" TAG_MSG_HAVE_PIC ")");
        if (mes->Flag & MESSAGE_HAVE_TEX)
                printf("(" TAG_MSG_HAVE_TEX ")");
        if (mes->Flag & MESSAGE_HAVE_TUB)
                printf("(" TAG_MSG_HAVE_TUB ")");
        if (mes->Flag & (MESSAGE_HAVE_URL | MESSAGE_HAVE_PICTURE | MESSAGE_HAVE_TEX | MESSAGE_HAVE_TUB))
                printf(" ");
        if (mes->Flag & MESSAGE_HAVE_BODY)
                printf("(" TAG_MSG_HAVE_BODY ")");
        else
                printf("(" TAG_MSG_HAVE_NO_BODY ")");
        printf("</span>");
        if (MESSAGE_INDEX_PRINT_ITS_URL & style)
                printf("</a>");
        if (mes->Readed)
                printf(" (%d)", mes->Readed);
        printf(" &mdash;" DESIGN_NOWRAP_END DESIGN_WRAP);

        if (currentdsm & CONFIGURE_host)  // only name
                printf(DESIGN_NOWRAP_START "%s &mdash;" DESIGN_NOWRAP_END DESIGN_WRAP, aname);
        else  // name and host
                printf(DESIGN_NOWRAP_START "%s" DESIGN_NOWRAP_END DESIGN_WRAP
                       DESIGN_NOWRAP_START "(%s) &mdash;" DESIGN_NOWRAP_END DESIGN_WRAP,
                       aname, mes->HostName);
        
        printf(DESIGN_NOWRAP_START "%s" DESIGN_NOWRAP_END, tm);

        if((mes->Flag & MESSAGE_IS_INVISIBLE) != 0) 
                printf("</strike>");

        printf("</span>");

        if(maxm_counter < mes->ViIndex) {maxm_counter = mes->ViIndex;}

        if(skipped != 0xffffffff && skipped != 0) {
                if(newmessmark) {
                        // does this message posted by registred user ?
                        if(lastmes.UniqUserID == 0) {
                                char *tmp = FilterBiDi(lastmes.AuthorName);
                                sprintf(aname, DESIGN_UNREGISTRED_NICK, tmp);
                                if (tmp)
                                        free(tmp);
                        } else {
                                char altnick[1000];
                                char *altnick_f;

                                if((currentdsm & CONFIGURE_nalt) == 0 && AltNames.NameToAltName(lastmes.UniqUserID, altnick)) {
                                        char *st;
                                        if(PrepareTextForPrint(altnick, &st, 1, MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE))
                                                strcpy(altnick, st);
                                } else
                                        strcpy(altnick, lastmes.AuthorName);

                                altnick_f = FilterBiDi(altnick);

                                // if this, is does this user view this message ;-) ?
                                if( ((currentdsm & CONFIGURE_onh) == 0) && ULogin.LU.ID[0] !=0 && lastmes.UniqUserID == ULogin.LU.UniqID ) {
                                        sprintf(aname, DESIGN_REGISTRED_OWN_NICK, altnick_f);
                                }
                                else {
                                        char *ts;
                                        // maybe user has selected some nicks to be displayed as detailed nicks?
                                        if(ULogin.LU.ID[0] != 0 && ((ts = strstr(ULogin.pfui->SelectedUsers, lastmes.AuthorName)) != NULL)) {
                                                sprintf(aname, DESIGN_SELECTEDUSER_NICK, altnick_f);
                                        }
                                        else {
                                                sprintf(aname, DESIGN_REGISTRED_NICK, altnick_f);
                                        }
                                }
                                if (altnick_f)
                                        free(altnick_f);
                        }
                        tm = ConvertTime(lastmes.Date);
                        char *aname_fbidi = FilterBiDi(aname);
                        printf(TAG_MSG_ROLLED_THREAD_MARKNEW, mes->ViIndex, skipped, newmessmark, MY_CGI_URL, lastmes.ViIndex, aname_fbidi, tm);
                        free(aname_fbidi);
                }
                else
                        printf(TAG_MSG_ROLLED_THREAD, mes->ViIndex, skipped);
        }

        if(mp != mes->MessageHeader) free(mp);
        return 1;
}

int DB_Base::printxmlmessage_in_index(SMessage *mes)
{

        // *******************************
        // BUG BUG with aname
        //////////////////////////////////
        char *mp = NULL, *pb;
        DWORD ff;

        if (!FilterBoardTags(mes->MessageHeader, &mp, 1, MAX_PARAMETERS_STRING, mes->Flag & MESSAGE_ENABLED_TAGS, &ff))
                mp = mes->MessageHeader;

        time_t t = mes->Date + 3600*DATETIME_DEFAULT_TIMEZONE;
        char *pubdate = asctime(gmtime(&t));
        pubdate[strlen(pubdate)-1]=0;
        
//=========================


        char *body;
        DWORD readed, xml_body_length = 0, tmp;
        

        if(mes->msize < XML_BODY_MAX) xml_body_length = mes->msize;
        else xml_body_length = XML_BODY_MAX;

        

        if((fb = wcfopen(F_MSGBODY, FILE_ACCESS_MODES_R)) == NULL)  printhtmlerrorat(LOG_UNABLETOLOCATEFILE, F_MSGBODY);
        if(wcfseek(fb, mes->MIndex, SEEK_SET) == -1) printhtmlerror();
        body = (char*)malloc(xml_body_length + 10);
        if((readed = wcfread(body, 1, xml_body_length + 2, fb)) < xml_body_length) printhtmlerror();
        wcfclose(fb);

        body[xml_body_length] = 0;

        if(xml_body_length != 0 && *body == 0) {
            char *ss = body;
            ss++;
            while(*ss) {
                        *(ss-1) = *(ss);
                        ss++;
            }
            *(ss-1) = *ss;
        }
        if(xml_body_length > 0) {
                //mes->Flag &= ~MESSAGE_ENABLED_TAGS;

                //printf("Flag %ld", mes->Flag);
                if(FilterBoardTags(body, &pb, 0, MAX_PARAMETERS_STRING, mes->Flag | BOARDTAGS_EXPAND_XMLEN , &tmp) == 0)
                        pb = body;
        
        }

        //======================

        printf("<item>\n");
        printf("<link>%s?read=%lu</link>\n", GetBoardUrl(), mes->ViIndex);
        printf("<guid>%s?read=%lu</guid>\n", GetBoardUrl(), mes->ViIndex);
        printf("<comments>%s?read=%lu</comments>\n", GetBoardUrl(), mes->ViIndex);
        printf("<pubDate>%s</pubDate>\n",pubdate);
        char *aname_fbidi = FilterBiDi(mes->AuthorName);
        printf("<author><![CDATA[%s@%s]]></author>\n", aname_fbidi, mes->HostName);
        if (aname_fbidi)
                free(aname_fbidi);

        char *aheader = FilterBiDi(mp);
#if        TOPICS_SYSTEM_SUPPORT
        if(mes->Topics <= TOPICS_COUNT - 1 && mes->Level == 0 && mes->Topics  != 0) {
                printf( "<category>%s</category>\n" ,Topics_List[mes->Topics]);
                printf("<title><![CDATA[%s]]></title>\n", aheader);
        }
        else {
                printf("<title><![CDATA[%s]]></title>\n", aheader);
        }
#else
        printf("<title><![CDATA[%s]]></title>\n", aheader);
#endif
        

        if(mes->msize > 0){
                printf("<description><![CDATA[%s]]></description>\n", pb);
        }
        
        printf("</item>\n\n");
        
        if (aheader)
                free(aheader);
        if(mp != mes->MessageHeader) free(mp);
        if(mes->msize > 0 && pb != body) free(pb);

        return 1;
        

}

int DB_Base::PrintandCheckMessageFavsExistandInv(SProfile_UserInfo *ui, DWORD viewinv, int *updated)
{

        SMessage msgs;
        DWORD fmpos, cnt = 0;
        int i;
        WCFILE *m;
        *updated = 0;

        printf("<div class=w>");
        printf("<P>");
        /* initializing */
        if((m = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        // search result entry
        for(i = PROFILES_FAV_THREADS_COUNT - 1; i + 1 > 0; i--) {
                
                if((fmpos = TranslateMsgIndex(ui->favs[i])) == NO_MESSAGE_CODE){
                        ui->favs[i] = 0;
                        *updated = 1;
                        continue;
                }
                if(wcfseek(m, fmpos, SEEK_SET) < 0) {
                        wcfclose(m);
                        return 0;
                }

                if(!fCheckedRead(&msgs, sizeof(SMessage), m)) printhtmlerror();
                if((msgs.Flag & MESSAGE_IS_INVISIBLE) && viewinv == 0) {
                        ui->favs[i] = 0;
                        *updated = 1;
                        continue;
                }
                printf("%s", DESIGN_break);
                cnt++;
                printf("%ld." "<A HREF=" MY_CGI_URL "?favdel=%ld target=\"_blank\">" DESIGN_FAVORITES_DEL_THREAD "</A> ",
                        cnt, ui->favs[i]);
                printhtmlmessage_in_index(&msgs, MESSAGE_INDEX_PRINT_ITS_URL | MESSAGE_INDEX_PRINT_BLANK_URL | MESSAGE_INDEX_DISABLE_ROLLED);
        }
        wcfclose(m);
        printf("</div>");
        printf("<P>");
        return cnt;
}

int DB_Base::printhtmlindexhron_wothreads()
{
        SMessage msgs;
        DWORD fpos, fmpos;
        WCFILE *f, *m;
        
        // initializing
        alrprn = 0;
        invflag = -1;
        collapsed = 0;
        newmessflag = 0;
        
        if((f = wcfopen(F_VINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if((m = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();

        printf("%s", DESIGN_open_dl);
        
        if(wcfseek(f, 0, SEEK_END) != 0) printhtmlerror();
        fpos = wcftell(f);

        do {
                if(fpos == 4) break;
                
                fpos-=sizeof(DWORD);

                if(wcfseek(f, fpos, SEEK_SET) < 0) printhtmlerror();
                if(!fCheckedRead(&fmpos, sizeof(DWORD), f))
                        printhtmlerror();

                if(fmpos == NO_MESSAGE_CODE) continue;

                if(wcfseek(m, fmpos, SEEK_SET) != 0)
                        printhtmlerror();
                if(!fCheckedRead(&msgs, sizeof(SMessage), m))
                        printhtmlerror();

                if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0 || !(msgs.Flag & MESSAGE_IS_INVISIBLE)){
                        printf("%s", DESIGN_break);
                        printhtmlmessage_in_index(&msgs, MESSAGE_INDEX_PRINT_ITS_URL | MESSAGE_INDEX_DISABLE_ROLLED);
                        alrprn++;
                }

                

        } while((currentlsel == 1 && msgs.Date > current_minprntime) ||
                (currentlsel == 2 && alrprn != currenttc + 1));

        printf("%s", DESIGN_close_dl);

        wcfclose(f);
        wcfclose(m);

        return 1;
}

int DB_Base::printhtmlindexhron_bythreads(DWORD mode)
{
        SMessageTable *buf;
        SMessage *msgs;
        DWORD rr, fmpos, shouldprint = 0xFFFFFFFF, skipped = 0;
        DWORD fipos;
        int LastLevel = -1;
        int firprn = 0;
        
        // initializing
        alrprn = 0;
        invflag = -1;
        collapsed = 0;
        newmessflag = 0;
        nt_counter = 0;
        nm_counter = 0;
        maxm_counter = 0;
        
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL)
                printhtmlerrorat(LOG_UNABLETOLOCATEFILE, F_MSGINDEX);
        if((fi = wcfopen(F_INDEX, FILE_ACCESS_MODES_R)) == NULL)
                printhtmlerrorat(LOG_UNABLETOLOCATEFILE, F_INDEX);

        if(wcfseek(fi, 0, SEEK_END) != 0) printhtmlerror();
        
        buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE + 1);
        msgs = (SMessage *)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER + 1);

        fipos = wcftell(fi);

        // beginning thread envelope - div
        if((mode & PRINTMODE_XML) == 0) {
                if (curcolor) printf("%s", DESIGN_open_dl_white);
                else printf("%s", DESIGN_open_dl_grey);
                curcolor=!curcolor;
        }

        for(;;) {
                DWORD tord;
                if(fipos == 0) break;
                else {
                        if(fipos >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                                fipos = fipos - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                                tord = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        }
                        else {
                                tord = fipos;
                                fipos = 0;
                        }
                }
                
                if(wcfseek(fi, fipos, SEEK_SET) != 0) printhtmlerror();
                if((rr = wcfread(buf, 1, tord, fi))  != tord) printhtmlerror();
                
                signed long i = rr / sizeof(SMessageTable) - 1;
                
                while(i >= 0) {
                        if(buf[i].begin < buf[i].end ) {
                                // forward direction
                                fmpos = buf[i].begin;
                                if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
                                while(fmpos != (buf[i].end + 1)) {
                                        DWORD toread;
                                        if(buf[i].end - fmpos < READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                toread = buf[i].end - fmpos + 1;
                                                fmpos = fmpos + toread;
                                        }
                                        else {
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                                fmpos = fmpos + toread;
                                        }
                                        if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
                                        
                                        if(printhtmlbuffer(msgs, toread, PRINT_FORWARD, &LastLevel, &firprn, mode, shouldprint, skipped) == 0) {
                                                goto End_of_Prn;
                                        }
                                        
                                }
                        }
                        else {
                                // backward direction
                                DWORD toread;
                                fmpos = buf[i].begin + 1;
                                while(fmpos != buf[i].end) {
                                        if( fmpos - buf[i].end > READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                fmpos = fmpos - READ_MESSAGE_HEADER*sizeof(SMessage);
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                        }
                                        else {
                                                toread = fmpos - buf[i].end;
                                                fmpos = buf[i].end;
                                        }
                                        
                                        if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
                                        if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
                                        
                                        if(printhtmlbuffer(msgs, toread, PRINT_BACKWARD, &LastLevel, &firprn, mode, shouldprint, skipped) == 0) {
                                                goto End_of_Prn;
                                        }
                                }
                        }
                        i--;
                }
        }
End_of_Prn:
        
        if((mode & PRINTMODE_XML) == 0) {
                for(int i = -1; i < LastLevel + 1; i++) printf("%s", DESIGN_close_dl);

                // index ends
                printf("</div>");
        }


        free(buf);
        free(msgs);
        wcfclose(fi);
        wcfclose(fm);
        return 1;        
}

DB_Base::DB_Base()
{
        fi = NULL;
        fb = NULL;
        fm = NULL;
        fv = NULL;
        maxm_counter = 0;
}

DB_Base::~DB_Base()
{
        
}

/* insert message "mes" with body "body" acording CFlags to board
 * REMARKS: mes->Host should be IP address, not a DNS name,
 * this function DOESN'T SET CURRENT DATE AND TIME
 * returns 0 if successfull, otherwise error code returned
 */
int DB_Base::DB_InsertMessage(struct SMessage *mes, DWORD root, WORD msize, char** body,
                                                          DWORD CFlags, char *passw, char **banreason)
{
        DWORD fp,fl;
        DWORD ri,fisize, rd;
        SMessageTable *buf;
        SMessage *msg;
        void *tmp;
        signed long i = 0;
        int code;
        int re; // reply flag
        DWORD MFlags, msigned = 0;

        CProfiles *uprof = NULL;
        /* posting user SProfile_UserInfo absoulte index in profile database */
        DWORD Uind = 0xFFFFFFFF;

        /****** check User ******/
        SProfile_UserInfo UI;

        if(passw != NULL && *passw != 0) {
                uprof = new CProfiles();
                if(uprof->errnum != PROFILE_RETURN_ALLOK) {
#if ENABLE_LOG >= 1
                        print2log("Error working with profiles database (init)");
#endif
                        printhtmlerror();
                }
                int opres = uprof->GetUserByName(mes->AuthorName, &UI, NULL, &Uind);
                if((opres != PROFILE_RETURN_ALLOK) || (strcmp(UI.password, passw) != 0)) {
                        delete uprof;
                        return MSG_CHK_ERROR_INVALID_PASSW;
                }
        }
        else {
                if(ULogin.LU.ID[0] != 0) {
                        //
                        //        User was already logged in
                        //
                        memcpy(&UI, ULogin.pui, sizeof(UI));
                        strcpy(mes->AuthorName, UI.username);
                        Uind = ULogin.LU.SIndex;
                }
                else {
                        char *st;

                        /* default user */
                        UI.right = DEFAULT_NOBODY_RIGHT;
                        UI.UniqID = 0;

                        //        check user name length ;-)
                        if(FilterBoardTags(mes->AuthorName, &st, 1, AUTHOR_NAME_LENGTH - 1, BOARDTAGS_TAG_PREPARSE, &fp) == 0) {
                                return MSG_CHK_ERROR_NONAME;
                        }
                        else {
                                strcpy(mes->AuthorName, st);
                                free(st);
                        }
                        if(strlen(mes->AuthorName) == 0 || strcmp(mes->AuthorName, " ") == 0) return MSG_CHK_ERROR_NONAME;
#if POST_ALLOW_UNDER_SAME_NICK == 0
                        uprof = new CProfiles();
                        if(uprof->GetUserByName(mes->AuthorName, NULL, NULL, NULL) == PROFILE_RETURN_ALLOK) {
                                delete uprof;
                            return MSG_CHK_ERROR_INVALID_PASSW;
                        }
#endif
                }
        }

// Check security rights for logged users
                if( 
                        !(UI.right & USERRIGHT_SUPERUSER) && 
                        !((UI.right & USERRIGHT_CREATE_MESSAGE) && root) &&
                        !((UI.right & USERRIGHT_CREATE_MESSAGE_THREAD) && root == 0)) {
                        return MSG_CHK_ERROR_BANNED;
                }
                                                                                                
        if(UI.Flags & PROFILES_FLAG_HAVE_SIGNATURE) {
                // set "signed" flag in message
                msigned = 1;
        }


        if(UI.right & USERRIGHT_ALLOW_HTML) CFlags = CFlags | MSG_CHK_ALLOW_HTML;
        // set poster ID of message
        mes->UniqUserID = UI.UniqID;

        // message header, banned address and spelling message check
        switch (code = CheckSpellingBan(mes, body, banreason, CFlags, &MFlags, UI.UniqID)) {
                case MSG_CHK_ERROR_PASSED:
                        break;
                default:
                        return code;
        }

        /* increase postcount if posting as registred user */
        if(Uind != 0xFFFFFFFF) {
                if(!uprof) uprof = new CProfiles();        // if it's logged in user
                
                UI.postcount++;
                UI.lastIP = mes->IPAddr;
                UI.LoginDate = time(NULL);
                
                if(uprof->SetUInfo(Uind, &UI) == 0) {
                        delete uprof;
#if ENABLE_LOG >= 1
                print2log("Call to CProfiles::SetUInfo failed at DB_Base::DB_InsertMessage(), line %d (Update user information for %s)",
                    __LINE__, UI.username);
#endif
                        printhtmlerror();
                }
        }
        if(uprof != NULL) delete uprof;


        if(*body != NULL && **body != 0)
                msize = (WORD)(strlen(*body) + 1);
        else
                msize = 0;

        // tuning some columns of the message stucture
        mes->Readed = 0;
        mes->msize = msize;
        mes->Level = 0;
        mes->Flag = 0;
        mes->ParentThread = 0;

        /************ set flags of message ************/
        /* tune [pic] [url] message flags and other message flags */
        mes->Flag = MFlags;

        // set "signed" flag
        if(msigned && (CFlags & MSG_CHK_DISABLE_SIGNATURE) == 0)
                mes->Flag |= MESSAGE_WAS_SIGNED;
        
        if (msize > 0)
                mes->Flag |= MESSAGE_HAVE_BODY;

        if((CFlags & MSG_CHK_ENABLE_EMAIL_ACKNL) && Uind != 0xFFFFFFFF)
                mes->Flag |= MESSAGE_MAIL_NOTIFIATION;
        /**********************************************/

        // tune message level and parent thread, if it's reply
        // *** NEW *** : also tune Topic (if only one topic per thread supported)
        if(root != 0) {
                DWORD viroot = root;
                re = 1; // set "reply" flag

                // get real index
                if ( (root = TranslateMsgIndex(root)) == NO_MESSAGE_CODE)
                        return MSG_CHK_ERROR_INVALID_NUMBER;
                
                // read parent message
                if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
                msg = (SMessage *)malloc(sizeof(SMessage));
                if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
                if(!fCheckedRead(msg, sizeof(SMessage), fm)) printhtmlerror();
                wcfclose(fm);

                // check whether post is allowed
                if ((UI.right & USERRIGHT_SUPERUSER) == 0) {
                        if (msg->Flag & MESSAGE_IS_CLOSED) {
                                free(msg);
                                return MSG_CHK_ERROR_CLOSED;
                        }
                        if (msg->Flag & MESSAGE_IS_INVISIBLE) {
                                free(msg);
                                return MSG_CHK_ERROR_INVISIBLE;
                        }
                }

                // tune level of message
                mes->Level = (WORD)(msg->Level + 1);

                // tune parent thread
                if(msg->Level > 0)
                        mes->ParentThread = msg->ParentThread;
                else
                        mes->ParentThread = root;

                // tune topic
                mes->Topics = msg->Topics;

                // tune flags (inheritance)
                mes->Flag |= ((msg->Flag & (MESSAGE_IS_INVISIBLE | MESSAGE_COLLAPSED_THREAD | MESSAGE_IS_CLOSED)));

                // send mail ackn if required
                {
                        // 1. Get user and email
                        CProfiles prof;
                        SProfile_UserInfo ui;
                        SProfile_FullUserInfo fui;
                        char subj[1000];
                        char bdy[100000];

                        fui.AboutUser = NULL;

                        if(msg->UniqUserID != 0 && prof.GetUserByName(msg->AuthorName, &ui, &fui, NULL) == PROFILE_RETURN_ALLOK &&
                                ((msg->Flag & MESSAGE_MAIL_NOTIFIATION) || (ui.Flags & PROFILES_FLAG_ALWAYS_EMAIL_ACKN)) ) {
                                char *pb, *pb1, *pb2;
                                char *pb_f, *pb1_f;

                                if(!PrepareTextForPrint(mes->MessageHeader, &pb, 1, (mes->Flag & MESSAGE_ENABLED_TAGS) | BOARDTAGS_CUT_TAGS)) {
                                        pb = (char*)malloc(strlen(mes->MessageHeader) + 1);
                                        strcpy(pb, mes->MessageHeader);
                                }
                                
                                pb_f = FilterBiDi(pb);
                                sprintf(subj, MAILACKN_REPLY_SUBJECT, pb_f);
                                free(pb);
                                if (pb_f)
                                        free(pb_f);

                                if(!PrepareTextForPrint(mes->MessageHeader, &pb, 1, mes->Flag | BOARDTAGS_EXPAND_ENTER)) {
                                        pb = (char*)malloc(strlen(mes->MessageHeader) + 1);
                                        strcpy(pb, mes->MessageHeader);
                                }
                                if(!PrepareTextForPrint(msg->MessageHeader, &pb1, 1, mes->Flag | BOARDTAGS_EXPAND_ENTER)) {
                                        pb1 = (char*)malloc(strlen(msg->MessageHeader) + 1);
                                        strcpy(pb1, msg->MessageHeader);
                                }
                                if(!PrepareTextForPrint(*body, &pb2, 0, mes->Flag | BOARDTAGS_EXPAND_ENTER | BOARDTAGS_PURL_ENABLE)) {
                                        pb2 = (char*)malloc(strlen(*body) + 1);
                                        strcpy(pb2, *body);
                                }

                                char *root_aname = FilterBiDi(msg->AuthorName);
                                char *aname = FilterBiDi(mes->AuthorName);
                                pb_f = FilterBiDi(pb);
                                pb1_f = FilterBiDi(pb1);
                                sprintf(bdy, MAILACKN_REPLY_BODY, root_aname, aname, pb1_f,  pb_f, pb2, GetBoardUrl(), viroot);
                                
                                wcSendMail(fui.Email, subj, bdy);

                                print2log("Mailackn was sent to %s", fui.Email);

                                free(pb);
                                free(pb1);
                                free(pb2);
                                if (pb_f)
                                        free(pb_f);
                                if (pb1_f)
                                        free(pb1_f);
                                if (root_aname)
                                        free(root_aname);
                                if (aname)
                                        free(aname);
                        }
                        if(fui.AboutUser) free(fui.AboutUser);
                }
                free(msg);
        }
        else {
                re = 0;
        }
        
        if((fb = wcfopen(F_MSGBODY, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerrorat(LOG_UNABLETOLOCATEFILERW, F_MSGBODY);
        // ******* lock fb *******
        lock_file(fb);

        // get free space and set file pointer
        CFreeDBFile fdb(F_FREEMBODY, 0);
        if(fdb.errnum != FREEDBFILE_ERROR_ALLOK) {
                unlock_file(fb);
                printhtmlerrorat(LOG_UNABLETOCREATEFILE, F_FREEMBODY);
        }
        if((rd = fdb.AllocFreeSpace(msize)) == 0xFFFFFFFF) {
                if(fdb.errnum != FREEDBFILE_ERROR_ALLOK) {
                        unlock_file(fb);
                        printhtmlerrorat(LOG_UNABLETOCREATEFILE, F_FREEMBODY);
                }
                
                if(wcfseek(fb, 0, SEEK_END) != 0) {
                        unlock_file(fb);
                        printhtmlerror();
                }
                rd = wcftell(fb);
                // in case if file is empty, to avoid msg_id=0
                if (rd == 0){
                        if(!fCheckedWrite(&rd, sizeof(DWORD), fb)) {
                                unlock_file(fb);
                                printhtmlerror();
                        }
                        rd = wcftell(fb);
                }
        }
        else {
                if(fdb.errnum != FREEDBFILE_ERROR_ALLOK) {
                        unlock_file(fb);
                        printhtmlerror();
                }
                if(wcfseek(fb, rd, SEEK_SET) != 0) {
                        unlock_file(fm);
                        printhtmlerror();
                }
        }
        
        // set index of body
        mes->MIndex = rd;
        if(!fCheckedWrite(*body, msize, fb)) {
                unlock_file(fb);
                printhtmlerror();
        }
        
        unlock_file(fb);
        // ******* unlock fb *******

        wcfclose(fb);

        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerror();
        // ****** lock fm ******
        lock_file(fm);
        
        // Allocate space for message header and save message index
        CFreeDBFile fdb1(F_FREEINDEX, sizeof(SMessage)); // Wasted block = sizeof(SMessage)
        if(fdb1.errnum != FREEDBFILE_ERROR_ALLOK) {
                unlock_file(fm);
                printhtmlerrorat(LOG_UNABLETOCREATEFILE, F_FREEINDEX);
        }
        if((fp = fdb1.AllocFreeSpace(sizeof(SMessage))) == 0xFFFFFFFF) {
                if(fdb1.errnum != FREEDBFILE_ERROR_ALLOK) {
                        unlock_file(fm);
                        printhtmlerrorat(LOG_UNABLETOCREATEFILE, F_FREEINDEX);
                }

                if(wcfseek(fm, 0, SEEK_END) != 0) {
                        unlock_file(fm);
                        printhtmlerror();
                }
                fp = wcftell(fm);
                // in case if file is empty, to avoid msg_id=0
                if (fp == 0){
                        if(!fCheckedWrite(&fp, sizeof(DWORD), fm)) {
                                unlock_file(fm);
                                printhtmlerror();
                        }
                        fp = wcftell(fm);
                }

        }
        else {
                if(fdb1.errnum != FREEDBFILE_ERROR_ALLOK) {
                        unlock_file(fb);
                        printhtmlerror();
                }
                if(wcfseek(fm, fp, SEEK_SET) != 0) {
                        unlock_file(fm);
                        printhtmlerror();
                }
        }

        // get msg index for new entry
        ri = AddMsgIndex(fp);
        // tune ViIndex in msg
        mes->ViIndex = ri;
        // write mes
        if(!fCheckedWrite(mes, sizeof(SMessage), fm)) {
                unlock_file(fm);
                printhtmlerror();
        }
        
        unlock_file(fm);
        // ****** unlock fm ******
        wcfclose(fm);

        if((fi = wcfopen(F_INDEX, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerror();
        // ****** lock fi ******
        lock_file(fi);

        // find index of root message
        if(re) {
                // temporary !!! should be added index support
                if(wcfseek(fi, 0, SEEK_END) != 0) {
                        unlock_file(fi);
                        printhtmlerror();
                }

                fisize = fl = wcftell(fi);
                buf = (SMessageTable *)malloc((sizeof(SMessageTable)*READ_MESSAGE_TABLE) + 1);

                while(fl > 0) {
                        DWORD toread;
                        if(fl >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                                fl = fl - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                                toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        }
                        else {
                                toread = fl;
                                fl = 0;
                        }
                
                        if(wcfseek(fi, fl, SEEK_SET) != 0) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
                        
                        if(!fCheckedRead(buf, toread, fi)) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
        
                        rd = i = (toread + 1) / sizeof(SMessageTable) - 1;
                        while(i>=0) {
                                if(M_IN(root, buf[i].begin, buf[i].end) || M_IN(root, buf[i].end, buf[i].begin)) {
                                        goto LB_end;
                                }
                                i--;
                        }
                }
                // NO MESSAGE
#if _DEBUG_ == 1
                print2log("Incorrect message DB - message not found");
#endif
                unlock_file(fi);
                wcfclose(fi);
                printhtmlerror();
                // --------------------------------------------

LB_end:
                tmp = malloc(fisize - fl + 10);
                if(wcfseek(fi, fl + ((i+1)*sizeof(SMessageTable)), SEEK_SET) == -1) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                // read end part of WCFILE
                DWORD readed;
                readed = wcfread(tmp, 1, fisize - fl + 10, fi);
                if(!wcfeof(fi)) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                if(wcfseek(fi, fl + (i*sizeof(SMessageTable)), SEEK_SET) == -1) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                if(buf[i].end < buf[i].begin) {
                        // write indexes up to message before root
                        fl = buf[i].begin;
                        if(buf[i].end < root) {
                                buf[i].begin = root - 1;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                        printhtmlerror();
                                }
                        }
                        if(root + sizeof(SMessage) == fp) {
                                // can join two messages with backward style
                                buf[i].begin = root;
                                buf[i].end = fp + sizeof(SMessage) - 1;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                        printhtmlerror();
                                }
                        }
                        else {                        
                                // write new message index
                                buf[i].begin = fp;
                                buf[i].end = fp + sizeof(SMessage) - 1;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                         printhtmlerror();
                                }
                                // write last messages
                                buf[i].begin = fl;
                                buf[i].end = root;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                        printhtmlerror();
                                }                                                
                        }
                }
                else {
                        if((buf[i].end + 1 == fp) && (root + sizeof(SMessage) == fp)) {
                                buf[i].end = fp + sizeof(SMessage) - 1;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                        printhtmlerror();
                                }
                        }
                        else {
                                fl = buf[i].begin;
                                if(buf[i].end + 1 != root + sizeof(SMessage)) {
                                        // write indexes up to root
                                        buf[i].begin = root + sizeof(SMessage);
                                        if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                                unlock_file(fi);
                                                printhtmlerror();
                                        }
                                }
                                // write new message index
                                buf[i].begin = fp;
                                buf[i].end = fp + sizeof(SMessage) - 1;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                        printhtmlerror();
                                }
                                // write last messages
                                buf[i].begin = fl;
                                buf[i].end = root + sizeof(SMessage) - 1;
                                if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                        unlock_file(fi);
                                        printhtmlerror();
                                }
                        }
                }
                // write end part of WCFILE
                if(!fCheckedWrite(tmp, readed, fi)) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                free(tmp);
        }
        else {

                // Increment main thread count
                IncrementMainThreadCount();

                DWORD fpos;
                if(wcfseek(fi, 0, SEEK_END) != 0) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                fpos = wcftell(fi);
                
                buf = (SMessageTable *)malloc(sizeof(SMessageTable));

                if(fpos > 0) {
                        // try to join message
                        fpos -= sizeof(SMessageTable);
                        if(wcfseek(fi, fpos, SEEK_SET) != 0) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
                        if(!fCheckedRead(buf, sizeof(SMessageTable), fi)) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
                        if((buf->begin > buf->end) && (buf->begin + 1 == fp)) {
                                buf->begin += sizeof(SMessage);
                        }
                        else {
                                if(((buf->end - buf->begin + 1)/sizeof(SMessage) == 1) && (buf->end + 1 == fp)) {
                                        buf->end = buf->begin;
                                        buf->begin = fp - 1 + sizeof(SMessage);
                                }
                                else {
                                        buf->begin = fp;
                                        buf->end = fp + sizeof(SMessage) - 1;
                                        fpos += sizeof(SMessageTable);
                                }
                        }
                }
                else {
                        // no join - just put it at the end of WCFILE
                        buf->begin = fp;
                        buf->end = fp + sizeof(SMessage) - 1;
                        //fpos += sizeof(SMessageTable);
                }

                if(wcfseek(fi, fpos, SEEK_SET) != 0) printhtmlerror();

                if(!fCheckedWrite(buf, sizeof(SMessageTable), fi)) {
                        unlock_file(fi);
                        printhtmlerror();
                }
        }

        // close all files and clean garbage database
        unlock_file(fi);
        // ****** unlock fi ******

        wcfclose(fi);
        free(buf);
        return MSG_CHK_ERROR_PASSED;
}

// delete messages with Level >= root.Level
// return 1 if successful, otherwise 0
int DB_Base::DB_DeleteMessages(DWORD root)
{
        DWORD rr;
        DWORD *msgsel;
        DWORD count;
        // select messages in root thread
        if(!SelectMessageThreadtoBuf(root, &msgsel, &count)) return 0;
        // delete selected messages
        for(rr = 0; rr < count; rr++) {
                DB_DeleteMessage(msgsel[rr]);
        }
        free(msgsel);
        return 1;
}

// return 1 if successfull, 0 otherwise
int DB_Base::DB_DeleteMessage(DWORD root)
{
        SMessage msg;
        SMessageTable *buf;
        DWORD fbsize, fbindex, oldroot;
        DWORD fl, fisize;
        int i = 0;
        void *tmp;
        oldroot = root;
        root = TranslateMsgIndex(root);
        if(root == NO_MESSAGE_CODE) return 0;

        //*******************************************
        //        Transaction should be started here
        //*******************************************

        //        Delete message reference
        if(DeleteMsgIndex(oldroot) == 0) return 0;
        //        Read message and save index of body and body size
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL) printhtmlerror();
        if(wcfseek(fm, root, SEEK_SET) < 0) printhtmlerror();
        if(!fCheckedRead(&msg, sizeof(SMessage), fm)) printhtmlerror();
        wcfclose(fm);
        fbsize = msg.msize;
        fbindex = msg.MIndex;
        //        Decrement thread count if it's main thread message
        if(msg.Level == 0) DecrementMainThreadCount();
        
        buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE + 1);
        
        // drop index in FI_INDEX
        
        if((fi = wcfopen(F_INDEX, FILE_ACCESS_MODES_RW)) == NULL) {
                printhtmlerror();
        }
        // ****** lock fi ******
        lock_file(fi);

        // find index of root message
        // temporary !!! should be added index support
        if(wcfseek(fi, 0, SEEK_END) != 0) {
                unlock_file(fi);
                printhtmlerror();
        }
        fisize = fl = wcftell(fi);
        while(fl > 0) {
                DWORD toread;
                if(fl >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                        fl = fl - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                }
                else {
                        toread = fl;
                        fl = 0;
                }
                if(wcfseek(fi, fl, SEEK_SET) != 0) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                if(!fCheckedRead(buf, toread, fi)) {
                        unlock_file(fi);
                        printhtmlerror();
                }
                i = (toread + 1) / sizeof(SMessageTable) - 1;
                while(i>=0) {
                        if(M_IN(root, buf[i].begin, buf[i].end) || M_IN(root, buf[i].end, buf[i].begin)) {
                                goto LB_MsgFound;
                        }
                        i--;
                }
        }
        // not found message
        unlock_file(fi);
        printhtmlerror();

LB_MsgFound:
        
        tmp = malloc(fisize - fl + 10);
        if(wcfseek(fi, fl + ((i+1)*sizeof(SMessageTable)), SEEK_SET) != 0) {
                unlock_file(fi);
                printhtmlerror();
        }
        // read end part of WCFILE
        DWORD readed;
        readed = wcfread(tmp, 1, fisize - fl + 10, fi);
        if(!wcfeof(fi)) {
                unlock_file(fi);
                printhtmlerror();
        }
        if(wcfseek(fi, fl + (i*sizeof(SMessageTable)), SEEK_SET) != 0) {
                unlock_file(fi);
                printhtmlerror();
        }
        
        oldroot = 0;
        if(buf[i].end < buf[i].begin) {
                // write indexes up to message before root
                fl = buf[i].begin;
                if(buf[i].end < root) {
                        oldroot = 1;
                        buf[i].begin = root - 1;
                        if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                oldroot = 1;                         
                                unlock_file(fi);
                                printhtmlerror();
                        }
                }
                // write last messages
                buf[i].begin = fl;
                buf[i].end = root + sizeof(SMessage);
                if(buf[i].begin + 1 != buf[i].end) {
                        oldroot = 1;
                        if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
                }
        }
        else {
                fl = buf[i].begin;
                if(buf[i].end + 1 != root + sizeof(SMessage)) {
                        oldroot = 1;
                        // write indexes up to root
                        buf[i].begin = root + sizeof(SMessage);
                        if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
                }
                // write last messages
                buf[i].begin = fl;
                buf[i].end = root - 1;
                if(buf[i].begin < buf[i].end) {
                        oldroot = 1;
                        if(!fCheckedWrite(&buf[i], sizeof(SMessageTable), fi)) {
                                unlock_file(fi);
                                printhtmlerror();
                        }
                }
        }

        if(!fCheckedWrite(tmp, readed, fi)) {
                unlock_file(fi);
                printhtmlerror();
        }

        free(tmp);

        if(!oldroot) fisize-=sizeof(SMessageTable);
        
        if(!oldroot) if(truncate(F_INDEX, fisize) != 0) {
                unlock_file(fi);
                printhtmlerror();
        }

        unlock_file(fi);
        // ****** unlock fi *******
        wcfclose(fi);
        
        free(buf);
        
        // mark messages as free space
        CFreeDBFile fdb(F_FREEINDEX, 0); // there is no wasted block !
        if(fdb.errnum != FREEDBFILE_ERROR_ALLOK)
                printhtmlerror();
        if(fdb.MarkFreeSpace(root, sizeof(SMessage)) != FREEDBFILE_ERROR_ALLOK)
                printhtmlerror();
        
        // free fb space
        CFreeDBFile fdb1(F_FREEMBODY, 0);
        if(fdb1.errnum != FREEDBFILE_ERROR_ALLOK)
                printhtmlerror();
        if(fdb1.MarkFreeSpace(fbindex, fbsize) != FREEDBFILE_ERROR_ALLOK)
                printhtmlerror();
        
        return MSG_CHK_ERROR_PASSED;
}

// if it executes w/o errors, it modifies mes->Flag and mes->ParentThread for you to generate correct redirect page
int DB_Base::DB_ChangeMessage(DWORD viroot, SMessage* mes, char **body, DWORD CFlags, char **banreason)
{
        int i;
        DWORD RFlags;
        DWORD omsize;
        WCFILE *fm;
        DWORD troot;
        SMessage msg;

        if ( (troot = TranslateMsgIndex(viroot)) == NO_MESSAGE_CODE)
                return MSG_CHK_ERROR_INVALID_NUMBER;
        if (!ReadDBMessage(troot, &msg))
                printhtmlerror();

        // go away if unreg
        if (ULogin.LU.ID[0] == 0)
                return MSG_CHK_ERROR_INVALID_PASSW;

        if((ULogin.pui->right & USERRIGHT_SUPERUSER) == 0) {
                if((ULogin.pui->right & USERRIGHT_MODIFY_MESSAGE) == 0 || ULogin.pui->UniqID != msg.UniqUserID)
                        return MSG_CHK_ERROR_EDIT_DENIED;
                if(msg.Flag & MESSAGE_IS_CLOSED)
                        return MSG_CHK_ERROR_CLOSED;
                if(msg.Flag & MESSAGE_IS_INVISIBLE)
                        return MSG_CHK_ERROR_INVISIBLE;
        }

        msg.MDate = time(NULL);
        strcpy(msg.MessageHeader, mes->MessageHeader);
        strcpy(msg.HostName, mes->HostName);
        msg.IPAddr = mes->IPAddr;

        if ((ULogin.pui->right & USERRIGHT_SUPERUSER) && strcmp(msg.AuthorName, mes->AuthorName)) {
                msg.UniqUserID = 0;
                strcpy(msg.AuthorName, mes->AuthorName);
        }

        if ( (i = CheckSpellingBan(&msg, body, banreason, CFlags, &RFlags)) != MSG_CHK_ERROR_PASSED)
                return i;

        // save size of old message body to decide later
        // if we must allocate another place for new body in DB
        omsize = msg.msize;

        //************ set sizes and flags of message ************/
        // save new message size
        if(*body && **body) {
                msg.msize = (WORD)(strlen(*body) + 1);
                msg.Flag |= MESSAGE_HAVE_BODY;
        } else {
                msg.msize = 0;
                msg.Flag &= ~MESSAGE_HAVE_BODY;
        }

        // tune [pic] [url] message flags and other message flags
        msg.Flag |= RFlags;

        if ((CFlags & MSG_CHK_ENABLE_EMAIL_ACKNL) && msg.UniqUserID)
                msg.Flag |= MESSAGE_MAIL_NOTIFIATION;
        else
                msg.Flag &= ~MESSAGE_MAIL_NOTIFIATION;

        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerror();

        // ********* lock FM *********
        lock_file(fm);

        if(wcfseek(fm, troot, SEEK_SET) != 0) {
                unlock_file(fm);
                printhtmlerror();
        }

        // enought size for write new body ?
        if(msg.msize > omsize) {
                // No, it's a pity we need to reallocate free space

                // mark body of message as free space
                CFreeDBFile fdb(F_FREEMBODY, 0);
                if(fdb.errnum != FREEDBFILE_ERROR_ALLOK) {
                        unlock_file(fm);
                        printhtmlerror();
                }
                if(fdb.MarkFreeSpace(msg.MIndex, omsize) != FREEDBFILE_ERROR_ALLOK) {
                        unlock_file(fm);
                        printhtmlerror();
                }
                
                // write new body
                if((fb = wcfopen(F_MSGBODY, FILE_ACCESS_MODES_RW)) == NULL) {
                        unlock_file(fm);
                        printhtmlerror();
                }
                // ********* lock fb *********
                lock_file(fb);

                // get free space and set WCFILE pointer
                if((msg.MIndex = fdb.AllocFreeSpace(msg.msize)) == 0xFFFFFFFF) {
                        // no free space - allocate it at the end of WCFILE
                        if(fdb.errnum != FREEDBFILE_ERROR_ALLOK) {
                                unlock_file(fm);
                                unlock_file(fb);
                                printhtmlerror();
                        }
                        
                        if(wcfseek(fb, 0, SEEK_END) != 0) {
                                unlock_file(fm);
                                unlock_file(fb);
                                printhtmlerror();
                        }
                        msg.MIndex = wcftell(fb);
                }
                else {
                        // free space found
                        if(wcfseek(fb, msg.MIndex, SEEK_SET) != 0) {
                                unlock_file(fm);
                                unlock_file(fb);
                                printhtmlerror();
                        }
                }

                if(!fCheckedWrite(*body, msg.msize, fb)) {
                        unlock_file(fb);
                        unlock_file(fm);
                        printhtmlerror();
                }
                unlock_file(fb);
                // ******* unlock fb *******
                wcfclose(fb);
                
        }
        else {
                // if the body present write it
                if(msg.msize) {
                        if((fb = wcfopen(F_MSGBODY, FILE_ACCESS_MODES_RW)) == NULL) {
                                unlock_file(fm);
                                printhtmlerror();
                        }
                        // ********* lock fb *********
                        lock_file(fb);

                        if(wcfseek(fb, msg.MIndex, SEEK_SET) != 0) {
                                unlock_file(fb);
                                unlock_file(fm);
                                printhtmlerror();
                        }
                        if(!fCheckedWrite(*body, msg.msize, fb)) {
                                unlock_file(fb);
                                unlock_file(fm);
                                printhtmlerror();
                        }
                        unlock_file(fb);
                        // ******** unlock fb ********
                        wcfclose(fb);
                }

                // more or equal to sizeof(SMessageBody) will left free?
                if(msg.msize < omsize) {
                        // mark it as free space
                        CFreeDBFile fdb(F_FREEMBODY, 0);
                        if(fdb.errnum != FREEDBFILE_ERROR_ALLOK) {
                                unlock_file(fm);
                                printhtmlerror();
                        }
                        if(fdb.MarkFreeSpace(msg.MIndex + msg.msize,
                                omsize - msg.msize) != FREEDBFILE_ERROR_ALLOK)
                        {
                                unlock_file(fm);
                                printhtmlerror();
                        }
                }
        }
        
        // write message header
        if(!fCheckedWrite(&msg, sizeof(SMessage), fm)) {
                unlock_file(fm);
                printhtmlerror();
        }
        
        unlock_file(fm);
        // unlock FM semaphore
        
        wcfclose(fm);

        mes->Flag = msg.Flag;
        mes->ParentThread = msg.ParentThread;

        return MSG_CHK_ERROR_PASSED;
}

/* Print message with body
 */
int DB_Base::PrintHtmlMessageBody(SMessage *msg, char *body)
{
        char *an, *pb, *ps = NULL;
        DWORD tmp;
        DWORD esm = 0;
        CProfiles prof;
#if SHOW_HOST_NAME
        int showhost = 1;
#else
        int showhost = 0;
#endif

        if((currentdsm & CONFIGURE_dsm) == 0)
                esm = MESSAGE_ENABLED_SMILES;

        if(!FilterBoardTags(msg->MessageHeader, &an, 1, MAX_PARAMETERS_STRING, msg->Flag, &tmp))
                an = msg->MessageHeader;

        if(msg->msize > 0)
                if(FilterBoardTags(body, &pb, 0, MAX_PARAMETERS_STRING, msg->Flag | esm | BOARDTAGS_PURL_ENABLE | BOARDTAGS_EXPAND_ENTER, &tmp) == 0)
                        pb = body;

        if(msg->Flag & MESSAGE_WAS_SIGNED) {
                SProfile_UserInfo ui;
                SProfile_FullUserInfo fui;
                if(prof.GetUserByName(msg->AuthorName, &ui, &fui, NULL) == PROFILE_RETURN_ALLOK &&
                        msg->UniqUserID == ui.UniqID)
                {
                        if(strlen(fui.Signature) > 0) {
                                // first of all - let's filter HTML !
                                char *st = FilterHTMLTags(fui.Signature, MAX_PARAMETERS_STRING);
                                if(st) {
                                        if(FilterBoardTags(st, &ps, 0, MAX_PARAMETERS_STRING,
                                                esm | MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE | BOARDTAGS_EXPAND_ENTER, &tmp) == 0)
                                        {
                                                ps = (char*)malloc(strlen(st) + 1);
                                                strcpy(ps, st);
                                        }
                                        free(st);
                                }
                        }
                }
        }

        printf(DESIGN_VIEW_THREAD_MSG_HEADER);

#if TOPICS_SYSTEM_SUPPORT || USER_FAVOURITES_SUPPORT == 2
        SMessage parmes;
        if (msg->ParentThread != 0 && msg->ParentThread != NO_MESSAGE_CODE) {
                if(!ReadDBMessage(msg->ParentThread, &parmes))
                        printhtmlerror();
        } else
                memcpy(&parmes, msg, sizeof(SMessage));
#endif

#if TOPICS_SYSTEM_SUPPORT
        if (parmes.Topics < TOPICS_COUNT)
                printf(DESIGN_VIEW_THREAD_TOPIC, Topics_List[parmes.Topics]);
        else
                printf(DESIGN_VIEW_THREAD_TOPIC, Topics_List[0]);
#endif

        char *an_f = FilterBiDi(an);
        printf("<BIG>%s</BIG>", an_f);
        if (an_f)
                free(an_f);

#if USER_FAVOURITES_SUPPORT
        DWORD favid;
#if USER_FAVOURITES_SUPPORT == 2
        favid = parmes.ViIndex;
#else
        favid = msg->ViIndex;
#endif
        if (ULogin.LU.ID[0] != 0) {
                DWORD result = prof.CheckandAddFavsList(ULogin.LU.SIndex, favid, 0);
                switch (result) {
                case PROFILE_RETURN_ALLOK:
                        printf("&nbsp;&nbsp;&nbsp;<A HREF=\"" MY_CGI_URL "?favadd=%ld\" target=_blank>"
                               DESIGN_FAVORITES_ADD_THREAD "</A>", favid);
                        break;
                case PROFILE_RETURN_ALREADY_EXIST:
                        printf("&nbsp;&nbsp;&nbsp;<A HREF=\"" MY_CGI_URL "?favdel=%ld\" target=_blank>"
                               DESIGN_FAVORITES_DEL_THREAD "</A>", favid);
                        break;
                case PROFILE_RETURN_UNKNOWN_ERROR:
                        break;
                }
        }
#endif

#if TOPICS_SYSTEM_SUPPORT
        // if we are not in preview mode and have admin right - let's print topic change form
        if( ((ULogin.LU.right & USERRIGHT_SUPERUSER) ||                        // modification by admin
                (ULogin.LU.ID[0] != 0 && msg->UniqUserID == ULogin.LU.UniqID &&
                (ULogin.LU.right & USERRIGHT_MODIFY_MESSAGE))) &&        // own message by user
                (msg->ParentThread != NO_MESSAGE_CODE)
                )
        {
                // print change topic form
                printf("\n<FORM><INPUT TYPE=HIDDEN NAME=\"ChangeTopic\" VALUE=\"%ld\">"
                        "<SELECT NAME=\"topic\">", parmes.ViIndex);
                for(DWORD i = 0; i < TOPICS_COUNT; i++) {
                        if(Topics_List_map[i] == msg->Topics) {
                                // define default choise
                                printf("<OPTION VALUE=\"%lu\"" LISTBOX_SELECTED ">%s\n", 
                                        Topics_List_map[i], Topics_List[Topics_List_map[i]]);
                        }
                        else {
                                printf("<OPTION VALUE=\"%lu\">%s\n", 
                                        Topics_List_map[i], Topics_List[Topics_List_map[i]]);
                        }
                }
                printf("</SELECT>&nbsp;&nbsp;<INPUT TYPE=SUBMIT NAME=\"ChangeTopic\" VALUE=\"Change\"></FORM>\n");
        }
#endif

        /* potential BUG here */
        char *aname = (char*)malloc(1000);
        Profile_UserName(msg->AuthorName, aname, msg->UniqUserID, 0);

        printf(DESIGN_VIEW_THREAD_MSG_SENT, MESSAGEMAIN_viewthread_sent, aname);

        free(aname);

        // print host name if requred
        if(showhost || (ULogin.LU.right & USERRIGHT_SUPERUSER)) {
                char tmp[10000], tmp2[100];
                unsigned char *aa = (unsigned char *)(&msg->IPAddr);
                strcpy(tmp, msg->HostName);
                //        print IP in numeric format for administrator
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) {
                        strcat(tmp, MESSAGEMAIN_viewthread_ipview);
                        sprintf(tmp2, "%u.%u.%u.%u", aa[0] & 0xff, aa[1] & 0xff, aa[2] & 0xff, aa[3] & 0xff);
                        strcat(tmp, tmp2);
                }
                printf(DESIGN_VIEW_THREAD_MSG_SENT2, tmp);
        }

        char *dt = ConvertFullTime((time_t)msg->Date);
        printf(DESIGN_VIEW_THREAD_DATE, MESSAGEMAIN_viewthread_date, dt);
        if(msg->MDate) {
                dt = ConvertFullTime((time_t)msg->MDate);
                printf(DESIGN_VIEW_THREAD_MDATE, MESSAGEMAIN_viewthread_mdate, dt);
        }

        printf(DESIGN_VIEW_THREAD_MSG_SENT3);

        //        Print body
        if(msg->msize > 0) {
		char *pb_f = FilterBiDi(pb);
                printf(DESIGN_VIEW_THREAD_BODY, pb_f);
                if (pb_f)
                        free(pb_f);
        }
        else printf(DESIGN_VIEW_THREAD_BODY, "");

        //        Print signature
        if(ps) {
                if((currentdsm & CONFIGURE_dsig) == 0) {
			char *ps_f = FilterBiDi(ps);
                        printf(DESIGN_VIEW_THREAD_SIGN, ps_f);
                        if (ps_f)
                                free(ps_f);
                }
                else {
                        printf(DESIGN_VIEW_THREAD_SIGN, MESSAGEMAIN_viewthread_sigdisabled);
                }
                free(ps);
        }

        printf("<BR><BR>");



        if(an != msg->MessageHeader) free(an);
        if(msg->msize > 0 && pb != body) free(pb);
        return 0;
}

// Be aware of adjusting flags:
// - is_xmlfp implies !only_body and print_body;
// - only_body implies print_body;
// in the above order.
char* DB_Base::PrintXmlMessageRoutine(DWORD num, int is_xmlfp, int only_body, int print_body)
{
        DWORD parnum = 0, index, tmp, i = 0;
        const size_t maxpartnum = 51;
        size_t partnum, partlen[maxpartnum], len = 0;
        const char *part[maxpartnum];
        char *s = NULL, *sp;
        char num_s[sizeof(num)*8/3 + 1], parnum_s[sizeof(num)*8/3 + 1];
        size_t num_s_len;
        char *header = NULL, *body = NULL, *author = NULL, *author_coded = NULL;
        char ctime[21], mtime[21];
        SMessage mes;
        int is_body;
#if TOPICS_SYSTEM_SUPPORT
        DWORD topicnum = 0;
#endif

        // adjust flags
        if (is_xmlfp) {
                only_body = 0;
                print_body = 1;
        } else if (only_body)
                print_body = 1;

        sprintf(num_s, "%lu", num);
        num_s_len = strlen(num_s);

        index = TranslateMsgIndexDel(num);
         if (index == 0) {
                char *r;
                if ( (r = (char*) malloc(XML_MES_STATUS_BASELEN +
                                         strlen(XML_MES_STATUS_DELETED) + num_s_len + 1)) == NULL)
                        printhtmlerror();
                sprintf(r, XML_MES_STATUS_TEMPLATE, num, XML_MES_STATUS_DELETED);
                return r;
        } else if (index == NO_MESSAGE_CODE) {
                char *r;
                if ( (r = (char*) malloc(XML_MES_STATUS_BASELEN +
                                         strlen(XML_MES_STATUS_NOTEXISTS) + num_s_len + 1)) == NULL)
                        printhtmlerror();
                sprintf(r, XML_MES_STATUS_TEMPLATE, num, XML_MES_STATUS_NOTEXISTS);
                return r;
        }

        if (!ReadDBMessage(index, &mes))
                printhtmlerror();

        if ((mes.Flag & MESSAGE_IS_INVISIBLE) && ((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0)) {
                char *r;
                if ( (r = (char*) malloc(XML_MES_STATUS_BASELEN +
                                         strlen(is_xmlfp ? XML_MES_STATUS_DELETED : XML_MES_STATUS_HIDED) +
                                         num_s_len + 1)) == NULL)
                        printhtmlerror();
                sprintf(r, XML_MES_STATUS_TEMPLATE, num, is_xmlfp ? XML_MES_STATUS_DELETED : XML_MES_STATUS_HIDED);
                return r;
        }

        if (print_body && (ULogin.LU.right & USERRIGHT_SUPERUSER) == 0 && CheckReadValidity(Nip, num)) {
                ++mes.Readed;
                if (!WriteDBMessage(index, &mes))
                        printhtmlerror();
        }

        if (!only_body) {
                char *header_to_cdata, *author_dec;
                DWORD tmp;

                if ( (parnum = getparent(num)) == NO_MESSAGE_CODE) {
                        char errmes[100];
                        sprintf(errmes, "error at %s:%d: parent of %s not found.", __FILE__, __LINE__, num_s);
                        print2log(errmes);
                        if (is_xmlfp) {
                                char *r;
                                if ( (r = (char*) malloc(XML_MES_STATUS_BASELEN +
                                                         strlen(XML_MES_STATUS_DELETED) + num_s_len + 1)) == NULL)
                                        printhtmlerror();
                                sprintf(r, XML_MES_STATUS_TEMPLATE, num, XML_MES_STATUS_DELETED);
                                return r;
                        } else
                                printhtmlerror();
                }
                sprintf(parnum_s, "%lu", parnum);

                if (!FilterBoardTags(mes.MessageHeader, &header_to_cdata, 1, MAX_PARAMETERS_STRING, mes.Flag, &tmp))
                        printhtmlerror();
                header = FilterCdata(header_to_cdata);
                free(header_to_cdata);

#if TOPICS_SYSTEM_SUPPORT
                if (parnum) {
                        SMessage rootmes;
                        if(!ReadDBMessage(mes.ParentThread, &rootmes))
                                printhtmlerror();
                        topicnum = rootmes.Topics;
                } else
                        topicnum = mes.Topics;
                
                if (topicnum >= TOPICS_COUNT)
                        topicnum = 0;
#endif
                author_dec = DefilterHTMLTags(mes.AuthorName);
                author = FilterCdata(author_dec);
                if (mes.UniqUserID)
                        author_coded = CodeHttpString(author_dec);
                free(author_dec);
                
                strftime(ctime, sizeof(ctime)/sizeof(ctime[0]), "%Y-%m-%dT%H:%M:%SZ", gmtime(&mes.Date));
                if (mes.MDate)
                        strftime(mtime, sizeof(mtime)/sizeof(mtime[0]), "%Y-%m-%dT%H:%M:%SZ", gmtime(&mes.MDate));
        }

        is_body = mes.msize && print_body;
        if (is_body) {
                char *body_to_filter, *body_to_cdata;
                if ( (body_to_filter = (char*) malloc(mes.msize)) == NULL)
                        printhtmlerror();
                if (!ReadDBMessageBody(body_to_filter, mes.MIndex, mes.msize))
                        printhtmlerror();
                if (!FilterBoardTags(body_to_filter, &body_to_cdata, 0, MAX_PARAMETERS_STRING,
                                     mes.Flag | BOARDTAGS_PURL_ENABLE | BOARDTAGS_EXPAND_ENTER, &tmp))
                        printhtmlerror();
                free(body_to_filter);
                body = FilterCdata(body_to_cdata);
                free(body_to_cdata);
        } else if (only_body) {
                char *r;
                const char *tmpl = "<message id=\"%lu\"/>";
                if ( (r = (char*) malloc(strlen(tmpl) - strlen("%lu") + num_s_len + 1)) == NULL)
                        printhtmlerror();
                sprintf(r, tmpl, num);
                goto end;
        }
        
        part[i++] = "<message id=\"";
        part[i++] = num_s;
        part[i++] = "\">";
        if (!only_body) {
                part[i++] = "<info>";
                part[i++] = "<date>";
                part[i++] = ctime;
                part[i++] = "</date>";
                if (mes.MDate) {
                        part[i++] = "<dateModified>";
                        part[i++] = mtime;
                        part[i++] = "</dateModified>";
                }
                if (parnum) {
                        part[i++] = "<parentId>";
                        part[i++] = parnum_s;
                        part[i++] = "</parentId>";
                }
                part[i++] = "<messageUrl>";
                part[i++] = "?read=";
                part[i++] = num_s;
                part[i++] = "</messageUrl>";
                part[i++] = "</info>";
                part[i++] = "<author>";
                part[i++] = "<name><![CDATA[";
                part[i++] = author;
                part[i++] = "]]></name>";
                if (mes.UniqUserID) {
                        part[i++] = "<id>";
                        part[i++] = author_coded;
                        part[i++] = "</id>";
                }
                part[i++] = "<host>";
                part[i++] = mes.HostName;
                part[i++] = "</host>";
                part[i++] = "<registered>";
                part[i++] = mes.UniqUserID ? "true" : "false";
                part[i++] = "</registered>";
                part[i++] = "</author>";
        }
        part[i++] = "<content>";
        if (!only_body) {
#if TOPICS_SYSTEM_SUPPORT
                part[i++] = "<category>";
                part[i++] = Topics_List[topicnum];
                part[i++] = "</category>";
#endif
                part[i++] = "<title><![CDATA[";
                part[i++] = header;
                part[i++] = "]]></title>";
        }
        if (is_body) {
                part[i++] = "<body><![CDATA[";
                part[i++] = body;
                part[i++] = "]]></body>";
        }
        if (!only_body &&
            ((mes.Flag & (MESSAGE_HAVE_URL | MESSAGE_HAVE_PICTURE | MESSAGE_HAVE_TEX | MESSAGE_HAVE_TUB)) ||
             (!is_xmlfp && (mes.Flag & (MESSAGE_IS_CLOSED | MESSAGE_IS_INVISIBLE | MESSAGE_COLLAPSED_THREAD))))) {
                part[i++] = "<tags>";
                if (!is_xmlfp) {
                        if (mes.Flag & MESSAGE_IS_CLOSED)
                                part[i++] = "<tag>closed</tag>";
                        if (mes.Flag & MESSAGE_IS_INVISIBLE)
                                part[i++] = "<tag>hided</tag>";
                        if (mes.Flag & MESSAGE_COLLAPSED_THREAD)
                                part[i++] = "<tag>collapsed</tag>";
                }
                if (mes.Flag & MESSAGE_HAVE_URL)
                        part[i++] = "<tag>" TAG_MSG_HAVE_URL "</tag>";
                if (mes.Flag & MESSAGE_HAVE_PICTURE)
                        part[i++] = "<tag>" TAG_MSG_HAVE_PIC "</tag>";
                if (mes.Flag & MESSAGE_HAVE_TEX)
                        part[i++] = "<tag>" TAG_MSG_HAVE_TEX "</tag>";
                if (mes.Flag & MESSAGE_HAVE_TUB)
                        part[i++] = "<tag>" TAG_MSG_HAVE_TUB "</tag>";
                part[i++] = "</tags>";
        }
        part[i++] = "</content>";
        part[i++] = "</message>";
        partnum = i;
        
        for (i = 0; i < partnum; ++i) {
                partlen[i] = strlen(part[i]);
                len += partlen[i];
        }

        if ( (s = (char*) malloc(len + 1)) ==  NULL)
                printhtmlerror();
        
        sp = s;
        for (i = 0; i < partnum; ++i) {
                strcpy(sp, part[i]);
                sp += partlen[i];
        }
        
end:
        free(header);
        free(body);
        free(author);
        free(author_coded);
        
        return s;
}

void DB_Base::PrintXmlMessage(DWORD num)
{
        printf(XML_START "%s", PrintXmlMessageRoutine(num));
}

void DB_Base::PrintXmlfpMessage(DWORD num)
{
        printf(XML_START "%s", PrintXmlMessageRoutine(num, 1));
}

void DB_Base::PrintXmlBody(DWORD num)
{
        printf(XML_START "%s", PrintXmlMessageRoutine(num, 0, 1));
}

void DB_Base::PrintXmlLastNumber()
{
        printf(XML_START "<lastMessageNumber>%lu</lastMessageNumber>", VIndexCountInDB());
}

void DB_Base::PrintXmlIndexRoutine(DWORD from, DWORD to, int is_xmlfp)
{
        DWORD i;
        char **msgs;

        if (from > to)
                printhtmlerror();

        if (to - from + 1 > XML_INDEX_MAXLEN)
                printhtmlerror();

        if ( (msgs = (char**) malloc((to - from + 1)*sizeof(msgs[0]))) == NULL)
                printhtmlerror();
        
        for (i = 0; i < to - from + 1; ++i)
                msgs[i] = PrintXmlMessageRoutine(from + i, is_xmlfp, 0, 0);

        printf(XML_START);
        printf("<messages>");
        for (i = 0; i < to - from + 1; ++i) {
                printf("%s", msgs[i]);
                free(msgs[i]);
        }
        printf("</messages>");

        free(msgs);
}

void DB_Base::PrintXmlIndex(DWORD from, DWORD to)
{
        PrintXmlIndexRoutine(from, to);
}

void DB_Base::PrintXmlfpIndex(DWORD from, DWORD to)
{
        PrintXmlIndexRoutine(from, to, 1);
}

// return 1 if valid, 0 otherwise
int DB_Base::DB_PrintMessageBody(DWORD root)
{
        SMessage msg;
        char *body;
        DWORD readed;

        //        Superuser access will not increment read count
        bool NeedReadedIncrement = ((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) &&
                (CheckReadValidity(Nip, root));

        //        Get real index
        root = TranslateMsgIndex(root);
        if(root ==  NO_MESSAGE_CODE) return 0;
        
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerror();
        
        
        // --- lock F_MSGINDEX file ---
        if(NeedReadedIncrement) lock_file(fm);
        
        if(wcfseek(fm, root, SEEK_SET) != 0)
        {
                if(NeedReadedIncrement) unlock_file(fm);
                printhtmlerror();
        }
        
        if(!fCheckedRead(&msg, sizeof(SMessage), fm))
        {
                if(NeedReadedIncrement) unlock_file(fm);
                printhtmlerror();
        }

        if(NeedReadedIncrement)
        {
                msg.Readed++;
                if(wcfseek(fm, root, SEEK_SET) != 0)
                {
                        unlock_file(fm);
                        printhtmlerror();
                }
                if(!fCheckedWrite(&msg, sizeof(SMessage), fm))
                {
                        unlock_file(fm);
                        printhtmlerror();
                }
                unlock_file(fm);
                wcfclose(fm);
        }
        // end of lock F_MSGINDEX file ---
        
        if((fb = wcfopen(F_MSGBODY, FILE_ACCESS_MODES_R)) == NULL)
                printhtmlerrorat(LOG_UNABLETOLOCATEFILE, F_MSGBODY);
        if(wcfseek(fb, msg.MIndex, SEEK_SET) == -1) printhtmlerror();
        body = (char*)malloc(msg.msize + 10);
        if((readed = wcfread(body, 1, msg.msize + 2, fb)) < msg.msize) printhtmlerror();
        body[readed] = 0;
        if(msg.msize != 0 && *body == 0) {
            char *ss = body;
            ss++;
            while(*ss) {
                *(ss-1) = *(ss);
                ss++;
            }
            *(ss-1) = *ss;
        }
        wcfclose(fb);

        PrintHtmlMessageBody(&msg, body);

        return 1;
}

/* print messages thread begining from message root, if root is not first message in the thread,
 * functions search for first message of the thread, and print thread from there
 * return 1 if valid, 0 otherwise
 */
int DB_Base::DB_PrintMessageThread(DWORD root)
{
        SMessageTable *buf;
        SMessage *msgs;
        DWORD rr, fipos, toread;
        DWORD fmpos, fl, oldroot;
        int LastLevel = 0;
        int i = 0;
        // translate virtual to real index, and check it
        root = TranslateMsgIndex(root);
        if(root == NO_MESSAGE_CODE) return 0; // invalid or nonexisting index
        
        buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE+1);
        msgs = (SMessage *)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER+1);
        
        // find where is main thread message
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
        if(!fCheckedRead(msgs, sizeof(SMessage), fm)) printhtmlerror();
        
        oldroot = msgs->ViIndex;
        
        if(msgs->Level != 0)
                rr = msgs->ParentThread;
        else
                rr = root;

        wcfclose(fm);
        
        /* set root index = index of main thread message */
        root = rr;
        
        if((fi = wcfopen(F_INDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if(wcfseek(fi, 0, SEEK_END) == -1) printhtmlerror();
        
        /* temporary !!! should be added index support */
        // find index in index file
        fl = wcftell(fi);
        
        buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE + 1);
        
        while(fl > 0) {
                DWORD toread;
                if(fl >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                        fl = fl - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                }
                else {
                        toread = fl;
                        fl = 0;
                }
                if(wcfseek(fi, fl, SEEK_SET) == -1) printhtmlerror();
                if(!fCheckedRead(buf, toread, fi)) printhtmlerror();
                i = (toread + 1) / sizeof(SMessageTable) - 1;
                while(i>=0) {
                        if(M_IN(root, buf[i].begin, buf[i].end) || M_IN(root, buf[i].end, buf[i].begin)) {
                                goto PT_Found;
                        }
                        i--;
                }
        }
        
        // not found in indexes - fatal error
        printhtmlerror();
        
PT_Found:
        fl = fl + (i + 1)*sizeof(SMessageTable);
        
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if(wcfseek(fi, fl, SEEK_SET) == -1) printhtmlerror();
        
        fipos = wcftell(fi);
        
        int czero = 0;
        int bp = 0;
        invflag = -1;
        collapsed = 0;
        
        printf(MESSAGEMAIN_in_this_thread);
        printf("%s", DESIGN_open_dl_white);

        
        for(;;) {
                if(fipos == 0) break;
                else {
                        if(fipos >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                                toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                                fipos = fipos - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        }
                        else {
                                toread = fipos;
                                fipos = 0;
                        }
                }
                if(wcfseek(fi, fipos, SEEK_SET) == -1) printhtmlerror();
                if((rr = wcfread(buf, 1, toread, fi)) % sizeof(SMessageTable) != 0) printhtmlerror();
                
                signed long i = rr / sizeof(SMessageTable) - 1;
                
                while(i >= 0) {
                        if(buf[i].begin < buf[i].end ) {
                                // forward direction
                                fmpos = buf[i].begin;
                                if(!czero) fmpos = root;                                
                                if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
                                while(fmpos != (buf[i].end + 1)) {
                                        DWORD toread;
                                        if(buf[i].end - fmpos < READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                toread = buf[i].end - fmpos + 1;
                                                fmpos = fmpos + toread;
                                        }
                                        else {
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                                fmpos = fmpos + toread;
                                        }
                                    //if(strcmp(getenv(QUERY_STRING), "read=1695642") == 0)  
                                        //  print4log("suxxxx: toread:%d, root:%d, fmpos:%d, beg:%d, end:%d", toread, root, fmpos, buf[i].begin, buf[i].end); 
                                        if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
                                        
                                        LastLevel = printThreadbuffer(msgs, toread, PRINT_FORWARD, fmpos - toread, LastLevel,
                                                &czero, oldroot, root, &bp);
                                        if(czero == 2) goto PT_Finish;
                                }
                        }
                        else {
                                // backward direction
                                DWORD toread;
                                fmpos = buf[i].begin + 1;
                                while(fmpos != buf[i].end) {
                                        if( fmpos - buf[i].end > READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                fmpos = fmpos - READ_MESSAGE_HEADER*sizeof(SMessage);
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                        }
                                        else {
                                                toread = fmpos - buf[i].end;
                                                fmpos = buf[i].end;
                                        }
                                        
                                        if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
                                        if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
                                        
                                        LastLevel = printThreadbuffer(msgs, toread, PRINT_BACKWARD, fmpos , LastLevel,
                                                &czero, oldroot, root, &bp);
                                        if(czero == 2) goto PT_Finish;
                                }
                        }
                        i--;
                }
        }
        
PT_Finish:
        for(i = -1; i < LastLevel; i++) printf("%s", DESIGN_close_dl);
        
        free(buf);
        free(msgs);
        wcfclose(fi);
        wcfclose(fm);
        return 1;
}

/* close posting messages to message thread begining from root
 * code = 1 - CLOSE, code = 0 - OPEN
 * return 1 if valid, 0 otherwise
 */
int DB_Base::DB_ChangeInvisibilityFlag(DWORD root, int invf)
{
        SMessage *msgs;
        // translate virtual to real index
        root = TranslateMsgIndex(root);
        if(root == NO_MESSAGE_CODE) return 0;
        msgs = (SMessage *)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER+1);
        
        // read message
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL) printhtmlerror();
        if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
        if(!fCheckedRead(msgs, sizeof(SMessage), fm)) printhtmlerror();
        
        // update flag
        if(invf == 1)
                msgs->Flag = msgs->Flag | MESSAGE_IS_INVISIBLE;
        else {
                if(msgs->Flag & MESSAGE_IS_INVISIBLE)
                        msgs->Flag = msgs->Flag - MESSAGE_IS_INVISIBLE; // should be XOR
        }

        // write message
        if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
        if(!fCheckedWrite(msgs, sizeof(SMessage), fm)) printhtmlerror();
        wcfclose(fm);
        free(msgs);
        return 1;
}

/* set collapsed flag on message root
 * behaviour depend of code:
 * if code == 0 - invert current value of flag
 * if code == 1 - set collapsed flag
 * if code == 0 - clean collapsed flag
 * return value : 1 if flag set, or 2 if flag not set 
 */
int DB_Base::DB_ChangeRollMessage(DWORD root, int code)
{
        SMessage *msgs;
        
        /* translate virtual to real index */
        root = TranslateMsgIndex(root);
        if(root == NO_MESSAGE_CODE)
                return 0;
        
        /* read message */
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL) {
                wcfclose(fm);
                return 0;
        }

        if(wcfseek(fm, root, SEEK_SET) != 0) {
                wcfclose(fm);
                return 0;
        }

        msgs = (SMessage *)malloc(sizeof(SMessage) + 1);

        if(!fCheckedRead(msgs, sizeof(SMessage), fm)) {
                wcfclose(fm);
                free(msgs);
                return 0;
        }

        /* update flag */
        DWORD flg = msgs->Flag;
        if(code == 0) {
                if(msgs->Flag & MESSAGE_COLLAPSED_THREAD)
                        msgs->Flag = msgs->Flag & (~MESSAGE_COLLAPSED_THREAD);
                else
                        msgs->Flag = msgs->Flag | MESSAGE_COLLAPSED_THREAD;
        }
        else
        if(code == 1) {
                msgs->Flag = msgs->Flag | MESSAGE_COLLAPSED_THREAD;
        }
        else
        if(code == 2) {
                msgs->Flag = msgs->Flag & (~MESSAGE_COLLAPSED_THREAD);
        }
        else {
                /* invalid code value */
                wcfclose(fm);
                free(msgs);
                return 0;
        }
        if(flg == msgs->Flag) {
                wcfclose(fm);
                free(msgs);
                return (!(msgs->Flag & MESSAGE_COLLAPSED_THREAD)) + 1;
        }

        // write message
        if(wcfseek(fm, root, SEEK_SET) != 0) printhtmlerror();
        if(!fCheckedWrite(msgs, sizeof(SMessage), fm)) {
                wcfclose(fm);
                free(msgs);
                return 0;
        }
        wcfclose(fm);
        int res = (!(msgs->Flag & MESSAGE_COLLAPSED_THREAD)) + 1;
        free(msgs);

        return res;
}

/* set "invisible" flag for messages with Level > root.Level in (thread)
 * code = 1 - CLOSE, code = 0 - OPEN
 * return 1 if valid, 0 otherwise
 */
int DB_Base::DB_ChangeCloseThread(DWORD root, int code)
{
        DWORD rr;
        DWORD *msgsel;
        DWORD count;

        /* select messages in root thread */
        if(!SelectMessageThreadtoBuf(root, &msgsel, &count)) return 0;

        /* change "invisible" flag on selected messages */
        for(rr = 0; rr < count; rr++) {
                DB_ChangeCloseMessage(msgsel[rr], code);
        }
        free(msgsel);

        return 1;
}

/* change collapsed thread bit on thread tmp */
int DB_Base::DB_ChangeRollThreadFlag(DWORD tmp)
{
        DWORD rr;
        DWORD *msgsel;
        DWORD count;

        /* select messages in root thread */
        if(!SelectMessageThreadtoBuf(tmp, &msgsel, &count)) return 0;

        /* change COLLAPSED_THREAD flag on selected messages */
        int code = 0;
        for(rr = 0; rr < count; rr++) {
                code = DB_ChangeRollMessage(msgsel[rr], code);
        }
        free(msgsel);

        return 1;
}

int DB_Base::SelectMessageThreadtoBuf(DWORD root, DWORD **msgsel, DWORD *mescnt)
{
        SMessageTable *buf;
        SMessage *msgs;
        DWORD rr, fipos, toread;
        DWORD fmpos, fl, EndLevel, viroot;
        int i;
        
        // array for storing selecting messages
        *mescnt = 0;
        *msgsel = (DWORD*)malloc(1);

        // save virtual index
        viroot = root;
        
        // translate virtual to real index, and check it
        root = TranslateMsgIndex(root);
        if(root == NO_MESSAGE_CODE) return 0; // invalid or nonexisting index
        
        buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE+1);
        msgs = (SMessage *)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER+1);
        
        // get EndLevel (current Level of root)
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
        if(!fCheckedRead(msgs, sizeof(SMessage), fm)) printhtmlerror();
        EndLevel = msgs->Level;
        wcfclose(fm);
        
        if((fi = wcfopen(F_INDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if(wcfseek(fi, 0, SEEK_END) == -1) printhtmlerror();
        
        // temporary !!! should be added index support
        // find index in index file
        fl = wcftell(fi);
        
        buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE + 1);
        
        while(fl > 0) {
                DWORD toread;
                if(fl >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                        fl = fl - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                }
                else {
                        toread = fl;
                        fl = 0;
                }
                if(wcfseek(fi, fl, SEEK_SET) == -1) printhtmlerror();
                if(!fCheckedRead(buf, toread, fi)) printhtmlerror();
                i = (toread + 1) / sizeof(SMessageTable) - 1;
                while(i>=0) {
                        if(M_IN(root, buf[i].begin, buf[i].end) || M_IN(root, buf[i].end, buf[i].begin)) {
                                goto PT_Found;
                        }
                        i--;
                }
        }
        
        // not found in indexes - fatal error
        return 0;
        
PT_Found:
        
        fl = fl + (i + 1)*sizeof(SMessageTable);
        
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if(wcfseek(fi, fl, SEEK_SET) == -1) printhtmlerror();
        
        fipos = wcftell(fi);
        
        //int czero = 0;
        // flag, showing if we reach ViIndex of root msg
        int reachviroot = 0;
        
        for(;;) {
                if(fipos == 0) break;
                else {
                        if(fipos >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
                                toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
                                fipos = fipos - READ_MESSAGE_TABLE*sizeof(SMessageTable);
                        }
                        else {
                                toread = fipos;
                                fipos = 0;
                        }
                }
                if(wcfseek(fi, fipos, SEEK_SET) == -1) printhtmlerror();
                if((rr = wcfread(buf, 1, toread, fi)) % sizeof(SMessageTable) != 0) printhtmlerror();
                signed long i = rr / sizeof(SMessageTable) - 1;
                while(i >= 0) {
                        
                        if(buf[i].begin < buf[i].end ) {
                                // forward direction
                                fmpos = buf[i].begin;
                                /*if(!czero) {
                                czero = 1;
                                fmpos = root;
                        }*/
                                if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
                                while(fmpos != (buf[i].end + 1)) {
                                        DWORD toread;
                                        if(buf[i].end - fmpos < READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                toread = buf[i].end - fmpos + 1;
                                                fmpos = fmpos + toread;
                                        }
                                        else {
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                                fmpos = fmpos + toread;
                                        }
                                        if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
                                        rr = toread/sizeof(SMessage);
                                        
                                        DWORD j;
                                        for(j = 0; j < rr; j++) {
                                                if(viroot == msgs[j].ViIndex) {
                                                        reachviroot = 1;
                                                }
                                                if(msgs[j].Level <= EndLevel && viroot != msgs[j].ViIndex && reachviroot) {
                                                        goto PT_Finish;
                                                }
                                                if(reachviroot) {
                                                        (*mescnt)++;
                                                        (*msgsel) = (DWORD*)realloc((*msgsel), (*mescnt)*sizeof(DWORD));
                                                        (*msgsel)[(*mescnt) - 1] = msgs[j].ViIndex;
                                                }
                                        }
                                }
                        }
                        else {
                                // backward direction
                                DWORD toread;
                                fmpos = buf[i].begin + 1;
                                /*if(!czero) {
                                czero = 1;
                                fmpos = root;
                        }*/
                                while(fmpos != buf[i].end) {
                                        if( fmpos - buf[i].end > READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                fmpos = fmpos - READ_MESSAGE_HEADER*sizeof(SMessage);
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                        }
                                        else {
                                                toread = fmpos - buf[i].end;
                                                fmpos = buf[i].end;
                                        }
                                        
                                        if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
                                        if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
                                        rr = toread/sizeof(SMessage);
                                        
                                        DWORD jj;
                                        for(jj=0; jj < rr; jj++) {
                                                DWORD j = rr - 1 - jj;
                                                if(viroot == msgs[j].ViIndex) {
                                                        reachviroot = 1;
                                                }

                                                if(msgs[j].Level <= EndLevel && viroot != msgs[j].ViIndex && reachviroot) {
                                                        goto PT_Finish;
                                                }
                                                if(reachviroot) {
                                                        (*mescnt)++;
                                                        (*msgsel) = (DWORD*)realloc((*msgsel), (*mescnt)*sizeof(DWORD));
                                                        (*msgsel)[(*mescnt) - 1] = msgs[j].ViIndex;
                                                }
                                        }
                                }
                        }
                        i--;
                }
        }
PT_Finish:
        wcfclose(fi);
        wcfclose(fm);
        free(buf);
        free(msgs);
        return 1;
}

// change "closed" flag on root message
// return 1 if successfull, otherwise 0
int DB_Base::DB_ChangeCloseMessage(DWORD root, int code)
{
        SMessage *msgs;
        // translate virtual to real index
        root = TranslateMsgIndex(root);
        
        msgs = (SMessage *)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER+1);
        
        // read message
        if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL) 
                printhtmlerror();
        // ******* lock FM *******
        lock_file(fm);

        if(wcfseek(fm, root, SEEK_SET) == -1) {
                unlock_file(fm);
                printhtmlerror();
        }
        if(!fCheckedRead(msgs, sizeof(SMessage), fm)) {
                unlock_file(fm);
                printhtmlerror();
        }
        // update flag
        if(!code) {
                if(msgs->Flag & MESSAGE_IS_CLOSED)
                        msgs->Flag = msgs->Flag - MESSAGE_IS_CLOSED;
        }
        else msgs->Flag = msgs->Flag | MESSAGE_IS_CLOSED;
        // write message
        if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
        if(!fCheckedWrite(msgs, sizeof(SMessage), fm)) {
                unlock_file(fm);
                printhtmlerror();
        }

        unlock_file(fm);
        // ******* unlock FM *******

        wcfclose(fm);
        free(msgs);
        return 1;
}

// change "invisible" flag on all messages with Level > root.Level
// return 1 if successfull, otherwise 0
int DB_Base::DB_ChangeInvisibilityThreadFlag(DWORD root, int invf)
{
        DWORD rr;
        DWORD *msgsel;
        DWORD count;
        // select messages in root thread
        if(!SelectMessageThreadtoBuf(root, &msgsel, &count)) return 0;
        //change "invisible" flag on selected messages
        for(rr = 0; rr < count; rr++) {
                DB_ChangeInvisibilityFlag(msgsel[rr], invf);
        }
        free(msgsel);

        return 1;
}

// return translated code, or NO_MESSAGE_CODE
DWORD DB_Base::TranslateMsgIndex(DWORD root)
{
        DWORD r;
        if(root !=0) {
                if((fv = wcfopen(F_VINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
                if(!(wcfseek(fv, root*sizeof(DWORD), SEEK_SET) != 0)) {
                        if(!fCheckedRead(&r, sizeof(DWORD), fv)) {
                                // reach end of file ?
                                r = NO_MESSAGE_CODE;
                        }
                }
                else r = NO_MESSAGE_CODE;
                wcfclose(fv);
        }
        else r = NO_MESSAGE_CODE;
        return r;
}

// as TranslateMsgIndex() but return 0 if the message was deleted
DWORD DB_Base::TranslateMsgIndexDel(DWORD root)
{
        return ::TranslateMsgIndexDel(root);
}


/* add message with real index root to virtual indexes and return
   virtual index of this message */
DWORD DB_Base::AddMsgIndex(DWORD root)
{
        DWORD r;
        // test if it's valid value
        //if(root == NO_MESSAGE_CODE) printhtmlerror();
        
        if((fv = wcfopen(F_VINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerror();
        // ******* lock FV *******
        lock_file(fv);

        if(wcfseek(fv, 0, SEEK_END) < 0) {
                unlock_file(fv);
                printhtmlerror();
        }
        r = wcftell(fv);
        // in case if file is empty, to avoid msg_id=0
        if (r == 0){
                if(!fCheckedWrite(&r, sizeof(DWORD), fv)) {
                        unlock_file(fv);
                        printhtmlerror();
                }
                r = wcftell(fv);
        }

        if(!fCheckedWrite(&root, sizeof(DWORD), fv)) {
                unlock_file(fv);
                printhtmlerror();
        }
        r = (r+1)/sizeof(DWORD);

        unlock_file(fv);
        // ******* unlock FV *******

        wcfclose(fv);
        return r;
}

// delete virtual message index
// return 1 if successfull, 0 otherwise
int DB_Base::DeleteMsgIndex(DWORD root)
{
        
        if((fv = wcfopen(F_VINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;
        // ******* lock FV *******
        lock_file(fv);

        if(wcfseek(fv, root*sizeof(DWORD), SEEK_SET) < 0) {
                unlock_file(fv);
                return 0;
        }
        root = NO_MESSAGE_CODE;
        if(!fCheckedWrite(&root, sizeof(DWORD), fv)) {
                unlock_file(fv);
                return 0;
        }
        unlock_file(fv);
        // ******* unlock FV *******

        wcfclose(fv);
        return 1;
}

DWORD DB_Base::VIndexCountInDB()
{
        return ::VIndexCountInDB();
}

DWORD DB_Base::MessageCountInDB()
{
        return ( ( (DWORD)(((DWORD)Fsize(F_MSGINDEX))) )/sizeof(SMessage) );
}

int DB_Base::ReadMainThreadCount(DWORD *root)
{
        FILE *f;
        int ret = 0;
        if((f = fopen(F_VINDEX, FILE_ACCESS_MODES_R)) == NULL)
                return ret;
        if(fread(root, 1, sizeof(DWORD), f) == sizeof(DWORD)) {
                ret = 1;
        }
        fclose(f);

        return ret;
}

int DB_Base::IncrementMainThreadCount()
{
        DWORD root;
        if((fv = wcfopen(F_VINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;
        // ******* lock FV *******
        lock_file(fv);

        if(!fCheckedRead(&root, sizeof(DWORD), fv)) {
                unlock_file(fv);
                return 0;
        }
        if(wcfseek(fv, 0, SEEK_SET) < 0) {
                unlock_file(fv);
                return 0;
        }
        root++;
        if(!fCheckedWrite(&root, sizeof(DWORD), fv)) {
                unlock_file(fv);
                return 0;
        }
        unlock_file(fv);
        // ******* unlock FV *******

        wcfclose(fv);

        return 1;
}

int DB_Base::DecrementMainThreadCount()
{
        DWORD root;
        if((fv = wcfopen(F_VINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;
        // ******* lock FV *******
        lock_file(fv);

        if(!fCheckedRead(&root, sizeof(DWORD), fv)) {
                unlock_file(fv);
                return 0;
        }
        if(wcfseek(fv, 0, SEEK_SET) < 0) {
                unlock_file(fv);
                return 0;
        }
        if(root) root--;
        if(!fCheckedWrite(&root, sizeof(DWORD), fv)) {
                unlock_file(fv);
                return 0;
        }
        unlock_file(fv);
        // ******* unlock FV *******

        wcfclose(fv);

        return 1;
}

DWORD DB_Base::getparent(DWORD root)
{
	SMessageTable *buf;
	SMessage *msgs;
	DWORD rr, fipos, toread;
	DWORD fmpos, fl, oldroot, parent = 0;
	WORD mlevel;
	int i = 0;
	int czero = 0, bp = 0;
	int success = 0;
	SMessage amsg;

	// translate virtual to real index, and check it
	root = TranslateMsgIndexDel(root);
	if (root == 0 || root == NO_MESSAGE_CODE)
                printhtmlerror();

	// find where is main thread message
	if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
	if(wcfseek(fm, root, SEEK_SET) == -1) printhtmlerror();
	if(!fCheckedRead(&amsg, sizeof(SMessage), fm)) printhtmlerror();
	wcfclose(fm);

	oldroot = amsg.ViIndex;
	mlevel = amsg.Level;

	if(amsg.Level != 0)
		rr = amsg.ParentThread;
	else
		return 0;

	/* set root index = index of main thread message */
	root = rr;
	if((fi = wcfopen(F_INDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
	if(wcfseek(fi, 0, SEEK_END) == -1) printhtmlerror();

	/* temporary !!! should be added index support */
	// find index in index file
	fl = wcftell(fi);

	buf = (SMessageTable *)malloc(sizeof(SMessageTable)*READ_MESSAGE_TABLE+1);
	msgs = (SMessage *)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER+1);

	while(fl > 0) {
		DWORD toread;
		if(fl >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
			fl = fl - READ_MESSAGE_TABLE*sizeof(SMessageTable);
			toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
		}
		else {
			toread = fl;
			fl = 0;
		}
		if(wcfseek(fi, fl, SEEK_SET) == -1) printhtmlerror();
		if(!fCheckedRead(buf, toread, fi)) printhtmlerror();
		i = (toread + 1) / sizeof(SMessageTable) - 1;
		while(i>=0) {
			if(M_IN(root, buf[i].begin, buf[i].end) || M_IN(root, buf[i].end, buf[i].begin)) {
				goto TT_Found;
			}
			i--;
		}
	}

	// not found in indexes - fatal error
	printhtmlerror();
 TT_Found:
	fl = fl + (i + 1)*sizeof(SMessageTable);

	if((fm = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
	if(wcfseek(fi, fl, SEEK_SET) == -1) printhtmlerror();

	fipos = wcftell(fi);


	for(;;) {
		if(fipos == 0) break;
		else {
			if(fipos >= READ_MESSAGE_TABLE*sizeof(SMessageTable)) {
				toread = READ_MESSAGE_TABLE*sizeof(SMessageTable);
				fipos = fipos - READ_MESSAGE_TABLE*sizeof(SMessageTable);
			}
			else {
				toread = fipos;
				fipos = 0;
			}
		}
		if(wcfseek(fi, fipos, SEEK_SET) == -1) printhtmlerror();
		if((rr = wcfread(buf, 1, toread, fi)) % sizeof(SMessageTable) != 0) printhtmlerror();

		signed long i = rr / sizeof(SMessageTable) - 1;
		while(i >= 0) {
			if(buf[i].begin < buf[i].end ) {
				// forward direction
				fmpos = buf[i].begin;
				if(!czero) fmpos = root;
				if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
				while(fmpos != (buf[i].end + 1)) {
					DWORD toread;
					if(buf[i].end - fmpos < READ_MESSAGE_HEADER*sizeof(SMessage)) {
						toread = buf[i].end - fmpos + 1;
						fmpos = fmpos + toread;
					}
					else {
						toread = READ_MESSAGE_HEADER*sizeof(SMessage);
						fmpos = fmpos + toread;
					}
					if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
					for (DWORD j = 0; j < toread/sizeof(SMessage); j++) {
						if(fmpos - toread + j*sizeof(SMessage) == root)
							bp = 1;
						if (bp) {
							if (msgs[j].Level == 0 && ++czero == 2)
								goto TT_Finish;
							if (msgs[j].Level == mlevel - 1)
								parent = msgs[j].ViIndex;
							if (msgs[j].ViIndex == oldroot) {
								success = 1;
								goto TT_Finish;
							}
						}
					}
				}
			} else {
				// backward direction
                                DWORD toread;
                                fmpos = buf[i].begin + 1;
                                while(fmpos != buf[i].end) {
                                        if( fmpos - buf[i].end > READ_MESSAGE_HEADER*sizeof(SMessage)) {
                                                fmpos = fmpos - READ_MESSAGE_HEADER*sizeof(SMessage);
                                                toread = READ_MESSAGE_HEADER*sizeof(SMessage);
                                        }
                                        else {
                                                toread = fmpos - buf[i].end;
                                                fmpos = buf[i].end;
                                        }
                                        
                                        if(wcfseek(fm, fmpos, SEEK_SET) == -1) printhtmlerror();
					if(!fCheckedRead(msgs, toread, fm)) printhtmlerror();
					for (DWORD jj = 0; jj < toread/sizeof(SMessage); jj++) {
                                                DWORD j = toread/sizeof(SMessage) - 1 - jj;
						if(fmpos + j*sizeof(SMessage) == root)
							bp = 1;
						if (bp) {
							if (msgs[j].Level == 0 && ++czero == 2)
								goto TT_Finish;
							if (msgs[j].Level == mlevel - 1)
								parent = msgs[j].ViIndex;
							if (msgs[j].ViIndex == oldroot) {
								success = 1;
								goto TT_Finish;
							}
						}
					}
                                }
			}
			i--;
		}
	}
 TT_Finish:
	free(buf);
	free(msgs);
	wcfclose(fi);
	wcfclose(fm);

	if (!success)
                return NO_MESSAGE_CODE;

	return parent;
}
