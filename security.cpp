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

                        SSpamQueue *mbuf = (SSpamQueue*)malloc(SPAMQUEUE_LENGTH*sizeof(SSpamQueue) + 1);
                        memset(mbuf, 0, sizeof(SSpamQueue)*SPAMQUEUE_LENGTH);

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

                        SReadQueue *mbuf = (SReadQueue*)malloc(READQUEUE_LENGTH*sizeof(SReadQueue) + 1);
                        memset(mbuf, 0, sizeof(SReadQueue)*READQUEUE_LENGTH);

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

//  This code makes available Marsaglia's highly portable generator
//  of uniformly distributed pseudo-random numbers.
//
//  The sequence of 24 bit pseudo-random numbers produced has a period
//  of about 2**144, and has passed stringent statistical tests for
//  randomness and independence.
//
//  Supplying a seed or seeds to init_rand(s1, s2) is optional.
//
//  The correspondence between pairs of seeds and generated sequences
//  of pseudo-random numbers is many-to-one.
//
//  References:
//      M G Harmon & T P Baker, ``An Ada Implementation of Marsaglia's
//      "Universal" Random Number Generator'', Ada Letters, late 1987.
//
//      G Marsaglia, ``Toward a universal random number generator'',
//      to appear in the Journal of the American Statistical Association.

//  output values are in the range 0..2^(24)-1

static const unsigned long  two_to_24     = 0x01000000;
static const unsigned short state_size    =         97;

static const unsigned short p             =        179;
static const unsigned short pm1           =      p - 1;
static const unsigned short q             =     p - 10;

static const unsigned long init_c         =     362436;
static const unsigned long cd             =    7654321;
static const unsigned long cm             =   16777213;

// The state of the generator is described by the following
// variables

static unsigned short  index_i;        //  [0..state_size-1]
static unsigned short  index_j;        //  [0..state_size-1]
static unsigned long   u[state_size];  //  the state array
static unsigned long   c;
static bool            init_done = false;

// This procedure initialises the state table u for a lagged
// Fibonacci sequence generator, filling it with random bits
// from a small multiplicative congruential sequence.    The
// auxilliaries c, index_i and index_j are also initialized.
// The seeds are transformed into an initial state in such a
// way that identical results are obtained on a wide variety
// of machines.

    // Return a value between 0 and size-1 inclusive.  This value
    // will be anyint itself if possible, otherwise another value
    // in the required interval.

static unsigned short col(short anyint, unsigned short size)
{
        short i = anyint;
        if(i < 0) i = - (i / 2);
        while (i >= size) i = i / 2;
        return i;
}

void init_rand(unsigned short seed_a, unsigned short seed_b)
{
        DWORD s,bit;
        WORD ii,jj,kk,mm;        // [0..p-1]
        WORD ll;                        // [0..q-1]
        short sd;

        sd = col(seed_a, pm1 * pm1);
        ii = 1 + sd / pm1; jj = 1 + sd % pm1;

        sd = col(seed_b, pm1 * q);
        kk = 1 + sd / pm1; ll = sd % q;

        if (ii == 1 && jj == 1 && kk == 1)
                ii = 2;

    for(short ind = 0; ind < state_size; ind++)
    {
        s = 0; bit = 1;

        do
        {
            mm = (((ii * jj) % p) * kk) % p;

            ii = jj; jj = kk; kk = mm;

            ll = (53 * ll + 1) % q;

            if ((ll * mm) & 0x0020)
                s += bit;

            bit <<= 1;
        } while(bit < two_to_24);

        u [ind] = s;
    }

    index_i = state_size - 1; index_j = state_size / 3; c  = init_c;

    init_done = true;
}

// Return uniformly distributed pseudo random numbers in the range
// 0..2^(24)-1 inclusive. There are 2^24 possible return values.

static DWORD rand24(void)
{
        DWORD temp;

        if(!init_done)
        {
                temp = (DWORD)time(NULL);
        init_rand((WORD)(temp >> 16), (WORD)temp);
        }

        c = (c < cd ? c + (cm - cd) : c - cd);

        temp = (u[index_i] -= u[index_j]);

        if(!index_i--)
                index_i = state_size - 1;

        if(!index_j--)
                index_j = state_size - 1;

        return (temp - c) & (two_to_24 - 1);
}

// Return uniformly distributed pseudo random number in the range
// 0..2^(32)-1 inclusive. There are 2^32 possible return values.

static BYTE buf[13];        // buffer for conversion from 24 to
                                                // 32 bit output
static unsigned short bno = 0;        // number of output values in buffer

DWORD rand32(void)
{
        if(!bno)
        {
                for(unsigned short i = 0; i < 12; i += 3)
                        *(DWORD*)(buf + i) = rand24();
                bno = 12;
        }
        bno -= 4;
        return *(DWORD*)(buf + 8 - bno);
}

/*
void MakeSHA(char *origstring, char *shastring)
{
        CSHA1 sha1;
        sha1.Reset();
        sha1.Update((UINT_8 *)origstring, strlen(origstring));
        sha1.Final();
        sha1.GetHash((UINT_8 *)shastring);
}
*/
