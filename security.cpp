/***************************************************************************
                          security.cpp  -  antispam&SSL and other secur.
                             -------------------
    begin                : Tue Apr 3 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "security.h"
#include "error.h"
#include "messages.h"

/* find out is IP and Msg in buf
 * also find latest date of record in list
 * if ip found it returns index of it in buf, otherwise -1 returned
 */
int CheckForIpAndMsgInQueue(DWORD ip, DWORD Msg, SReadQueue *buf, int n, int &Alatest)
{
        Alatest = 0; // Max time

        for(int i = 0; i < n; i++)
        {
                // also find latest date
                if(buf[Alatest].Date > buf[i].Date) Alatest = i;
                
                if(buf[i].IP == ip && buf[i].MsgIndex == Msg)
                {
                        return i;
                }
        }

        return -1;
}

/* */
int CheckReadValidity(DWORD IP, DWORD MsgIndex)
{
        WCFILE *f;
        SReadQueue buf[READQUEUE_LENGTH + 1];
        time_t t = time(NULL);
        int x, Alatest;

        if((f = wcfopen(F_ANTISPAM, FILE_ACCESS_MODES_RW)) == NULL)
        {
                if((f = wcfopen(F_ANTISPAM, FILE_ACCESS_MODES_CW)) != NULL)
                {
                        // create and save post list

                        lock_file(f);

                        // TODO: replace this sequence with a single truncate() call
                        SSpamQueue *mbuf = (SSpamQueue*)calloc(SPAMQUEUE_LENGTH, sizeof(SSpamQueue));
                        if (!mbuf) {
                                unlock_file(f);
                                wcfclose(f);
                                printhtmlerror();
                        }

                        if(!fCheckedWrite(mbuf, SPAMQUEUE_LENGTH*sizeof(SSpamQueue), f)) {
                                unlock_file(f);
                                wcfclose(f);
                                printhtmlerror();
                        }
                        free(mbuf);

                        // create and save read list

                        memset(buf, 0, sizeof(SReadQueue)*READQUEUE_LENGTH);

                        if(!fCheckedWrite(buf, READQUEUE_LENGTH*sizeof(SReadQueue), f)) {
                                unlock_file(f);
                                wcfclose(f);
                                printhtmlerror();
                        }

                        unlock_file(f);

                        wcfclose(f);

                        if((f = wcfopen(F_ANTISPAM, FILE_ACCESS_MODES_RW)) == NULL)
                                printhtmlerror();
                }
                else printhtmlerror();
        }

        lock_file(f);

        if(wcfseek(f, READQUEUE_PREFIX, SEEK_SET) != 0) {
                unlock_file(f);
                wcfclose(f);
                printhtmlerror();
        }

        if(!fCheckedRead(buf, READQUEUE_LENGTH*sizeof(SReadQueue), f)) {
                unlock_file(f);
                wcfclose(f);
                printhtmlerror();
        }

        if((x = CheckForIpAndMsgInQueue(IP, MsgIndex, buf, READQUEUE_LENGTH, Alatest)) != -1)
        {
                unlock_file(f);
                wcfclose(f);
                // return status = READ INVALID
                return 0;
        }

        // update: delete entry with most early date
        // and set our client information
        buf[Alatest].IP = IP;
        buf[Alatest].Date = t;
        buf[Alatest].MsgIndex = MsgIndex;
        
        if(wcfseek(f, READQUEUE_PREFIX, SEEK_SET) != 0) {
                unlock_file(f);
                wcfclose(f);
                printhtmlerror();
        }
        if(!fCheckedWrite(buf, READQUEUE_LENGTH*sizeof(SReadQueue), f)) {
                unlock_file(f);
                wcfclose(f);
                printhtmlerror();
        }
        unlock_file(f);
        wcfclose(f);
        // return status - READ VALID
        return 1;
}

/* find out is IP in buf
 * if ip found it returns index of it in buf, otherwise -1 returned
 */
int CheckForIpInQueue(DWORD ip, SSpamQueue *buf, int n, int &Alatest)
{
        Alatest = 0; // Max time

        for(int i = 0; i < n; i++)
        {
                // also find latest date
                if(buf[Alatest].Date > buf[i].Date) Alatest = i;
                
                if(buf[i].IP == ip)
                {
                        return i;
                }
        }

        return -1;
}


int MarkPostfromIPInvalid(DWORD IP)
{
        int Alatest;
        WCFILE *f;
        time_t t = time(NULL);
        SSpamQueue buf[SPAMQUEUE_LENGTH + 1];

        if((f = wcfopen(F_ANTISPAM, FILE_ACCESS_MODES_RW)) == NULL)
                printhtmlerror();

        lock_file(f);
        
        if(!fCheckedRead(buf, SPAMQUEUE_LENGTH*sizeof(SSpamQueue), f))
                printhtmlerror();

        Alatest = 0; //        Max time

        for(int i = 0; i < SPAMQUEUE_LENGTH; i++)
        {
                //        find earliest date
                if(buf[Alatest].Date > buf[i].Date) Alatest = i;
        }

        // update: delete entry with most early date
        buf[Alatest].IP = IP;
        buf[Alatest].Date = t;

        if(wcfseek(f, 0, SEEK_SET) != 0)
                printhtmlerror();
        if(!fCheckedWrite(buf, SPAMQUEUE_LENGTH*sizeof(SSpamQueue), f))
                printhtmlerror();

        unlock_file(f);
        wcfclose(f);

        return 0;
}

/* check if it valid enter of user with IP
 * return 1 if valid, 0 otherwise
 */
int CheckPostfromIPValidity(DWORD IP, int TimeInterval)
{
        WCFILE *f;
        SSpamQueue buf[SPAMQUEUE_LENGTH + 1];
        time_t t = time(NULL);
        int x, Alatest;

        if((f = wcfopen(F_ANTISPAM, FILE_ACCESS_MODES_RW)) == NULL)
        {
                if((f = wcfopen(F_ANTISPAM, FILE_ACCESS_MODES_CW)) != NULL)
                {
                        // create and save post list

                        lock_file(f);

                        memset(buf, 0, sizeof(SSpamQueue)*SPAMQUEUE_LENGTH);

                        if(!fCheckedWrite(buf, SPAMQUEUE_LENGTH*sizeof(SSpamQueue), f)) {
                                unlock_file(f);
                                wcfclose(f);
                                printhtmlerror();
                        }

                        // create and save read list
                        // TODO: replace this sequence with a single truncate() call
                        SReadQueue *mbuf = (SReadQueue*)calloc(READQUEUE_LENGTH, sizeof(SReadQueue));
                        if (!mbuf) {
                                unlock_file(f);
                                wcfclose(f);
                                printhtmlerror();
                        }

                        if(!fCheckedWrite(mbuf, READQUEUE_LENGTH*sizeof(SReadQueue), f)) {
                                unlock_file(f);
                                wcfclose(f);
                                printhtmlerror();
                        }
                        free(mbuf);

                        unlock_file(f);

                        wcfclose(f);

                        return 1;
                }
                else printhtmlerror();
        }
        
        lock_file(f);
        
        if(!fCheckedRead(buf, SPAMQUEUE_LENGTH*sizeof(SSpamQueue), f))
                printhtmlerror();

        unlock_file(f);
        wcfclose(f);

        if((x = CheckForIpInQueue(IP, buf, SPAMQUEUE_LENGTH, Alatest)) != -1)
        {
                if(buf[x].Date + TimeInterval > t && buf[x].Date < t )
                {
                        // return status - POST INVALID
                        return 0;
                }
        }

        // return status - POST VALID
        return 1;
}
