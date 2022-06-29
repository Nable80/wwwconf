/***************************************************************************
                      dbase.h  -  message database engine include
                         -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef DBASE_H_INCLUDED
#define DBASE_H_INCLUDED

#include "basetypes.h"
#include "speller.h"
#include "logins.h"
#include "colornick.h"

struct SMessageTable
{
        /* index ordering table */
        DWORD begin;
        DWORD end;
};

#define PRINTMODE_NULL                        0x0000
#define PRINTMODE_ALL_ROLLED        0x0001
#define PRINTMODE_XML                        0x0002

// IP address of user
extern char *Cip;
extern DWORD Nip;


/* cookie data */
extern DWORD                currentlsel;
extern DWORD                currenttc;
extern DWORD                currenttt;
extern DWORD                currenttv;
extern DWORD                currentss;
extern DWORD                currentdsm;
extern DWORD                currenttopics;
extern DWORD                currentlann;
extern DWORD                topicsoverride;
extern DWORD                currentlm;        // last message
extern DWORD                currentfm;        // first message
extern DWORD                currentlt;        // last threads
extern DWORD                currentft;        // first threads
extern int                currenttz;        

extern DWORD                currenttopics_map;

extern DWORD cookie_lsel;
extern DWORD cookie_tc;
extern DWORD cookie_tt;
extern DWORD cookie_tv;
extern DWORD cookie_ss;
extern DWORD cookie_dsm;
extern DWORD cookie_topics;
extern long cookie_tz;

extern char *cookie_seq;
extern char *cookie_name;

extern time_t current_minprntime;

/* shows if header of CGI script was printed */
extern int HPrinted;

extern CUserLogin ULogin;

#if USER_ALT_NICK_SPELLING_SUPPORT
extern CAltNamesParser AltNames;
#endif

/* ------------------------
 *  functions
 * ------------------------
 */
char *ConvertTime(time_t t);
char *ConvertFullTime(time_t t);
int IP2HostName(DWORD IP, char *hostname, int maxlen);

class DB_Base
{
protected:
        WCFILE        *fi,        // random access index file
                        *fm,        // message headers
                        *fb,        // message bodies
                        *fv,        // virtual index file
                        *ffm,        // free spaces in message headers
                        *ffb;        // free spaces in message bodies

        DWORD alrprn;                // count of already printed threads
        DWORD nt_counter;        // count of printed new threads
        DWORD nm_counter;        // count of printed new msgs
        DWORD maxm_counter; // last printed msgs
        /* fpr printhtmlindex */
        int invflag;        // "invisible thread was printed" flag
        int collapsed;        // "rolled thread was printed" flags
        int newmessflag;// "new message" mark preparing for collapsed threads
        SMessage pmes;        // for message preprinting
        SMessage lastmes;        // for last posted message in collapsed thread

        int SelectMessageThreadtoBuf(DWORD root, DWORD **msgsel, DWORD *msgcnt);
        
        int printhtmlbuffer(SMessage *buf, DWORD size, int p, int *ll, int *pr, DWORD mode, DWORD &shouldprint, DWORD &skipped);
        int printThreadbuffer(SMessage *buf, DWORD size, int p, DWORD fmpos, int ll, int *czero, DWORD selected, DWORD root, int *bp);
        void printhtmlindexhron_bythreads(DWORD mode);
        int printhtmlindexhron_wothreads();
        int printhtmlmessage_in_index(SMessage *mes, int style, DWORD skipped = 0xffffffff, int newmessmark = 0);
        int printxmlmessage_in_index(SMessage *mes);


        int DB_ChangeInvisibilityFlag(DWORD root, int invf);
        int DB_ChangeCloseMessage(DWORD root, int code);
        int DB_DeleteMessage(DWORD root);
        int DB_ChangeRollMessage(DWORD root, int code);

public:
        DB_Base();
        ~DB_Base();
        
        DWORD TranslateMsgIndex(DWORD root);
        DWORD TranslateMsgIndexDel(DWORD root);
        DWORD AddMsgIndex(DWORD root);
        int DeleteMsgIndex(DWORD root);
        DWORD VIndexCountInDB();

        DWORD MessageCountInDB();

        int IncrementMainThreadCount();
        int DecrementMainThreadCount();
        int ReadMainThreadCount(DWORD *root);
        
        int PrintHtmlMessageBody(SMessage *mes,  char *body);

        char* PrintXmlMessageRoutine(DWORD num, int is_xmlfp = 0, int only_body = 0, int print_body = 1);
        void PrintXmlMessage(DWORD num);
        void PrintXmlfpMessage(DWORD num);
        void PrintXmlBody(DWORD num);
        void PrintXmlLastNumber();
        void PrintXmlIndexRoutine(DWORD from, DWORD to, int is_xmlfp = 0);
        void PrintXmlIndex(DWORD from, DWORD to);
        void PrintXmlfpIndex(DWORD from, DWORD to);

        int DB_InsertMessage(struct SMessage *mes, DWORD root, DWORD msize, char** body, DWORD CFlags, char *passw, char **banreason);
        int DB_ChangeMessage(DWORD viroot, SMessage* mes, char **body, DWORD CFlags, char **banreason);
        int PrintandCheckMessageFavsExistandInv(SProfile_UserInfo *ui, DWORD viewinv, int *updated);
        
        int DB_DeleteMessages(DWORD root);
        int DB_ChangeInvisibilityThreadFlag(DWORD root, int invf);
        int DB_ChangeRollThreadFlag(DWORD tmp);
        int DB_ChangeCloseThread(DWORD root, int code);
        int DB_PrintMessageThread(DWORD root);
        void DB_PrintHtmlIndex(DWORD mtc);
        int DB_PrintMessageBody(DWORD root);
        void Profile_UserName(char *name, char *tostr, int reg, int doparsehtml = 1);
	DWORD getparent(DWORD);
};

#endif

