/***************************************************************************
                          security.h  -  description
                             -------------------
    begin                : Tue Apr 3 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef SECURITY_H_INCLUDED
#define SECURITY_H_INCLUDED

#include "basetypes.h"
#include "sha1.h"

// antispam codes
#define POST_MSG		1
#define REFRESH_INDEX	2
#define READ_MSG		3

#define SPAMQUEUE_PREFIX		 0
#define SPAMQUEUE_LENGTH		60

#define READQUEUE_PREFIX		60*sizeof(SSpamQueue)
#define READQUEUE_LENGTH		60

struct SSpamQueue
{
	DWORD IP;				// ip address
	time_t Date;			// date
};

struct SReadQueue
{
	time_t Date;	// date
	DWORD IP;		// IP-address
	DWORD MsgIndex; // message index
};

int MarkPostfromIPInvalid(DWORD IP, int TimeInterval);
int CheckPostfromIPValidity(DWORD IP, int TimeInterval);

int CheckReadValidity(DWORD IP, DWORD MsgIndex);

unsigned long rand32(void);

void init_rand(unsigned short seed_a = 1991, unsigned short seed_b = 9880);

void MakeSHA(char *origstring, char *shastring);

#endif
