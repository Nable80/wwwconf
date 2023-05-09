/***************************************************************************
                      hashindex.cpp  -  hashed indexes for string include
                             -------------------
    begin                : Sun Nov 5 2002
    copyright            : (C) 2001-2002 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef _HASHINDEX_H_INCLUDED_
#define _HASHINDEX_H_INCLUDED_

#include "basetypes.h"

#define HASHINDEX_ER_OK                                0
#define HASHINDEX_ER_ALREADY_EXIST        1
#define HASHINDEX_ER_NOT_FOUND                2
#define HASHINDEX_ER_IO_CREATE                3
#define HASHINDEX_ER_IO_READ                4
#define HASHINDEX_ER_IO_WRITE                5
#define HASHINDEX_ER_FORMAT                        6

#define HASHTAB_LEN 256
#define HASHINDEX_BLOCK_SIZE 2000
#define HASHINDEX_NULL        0xFFFFFFFF

typedef struct _HASHINDEX_BLOCKINFO {
        WORD Used;
        DWORD Next;
} HASHINDEX_BLOCKINFO, *PHASHINDEX_BLOCKINFO;

int AddStringToHashedIndex(const char *s, DWORD Index);

int GetIndexOfString(const char *s, DWORD *Index);

int DeleteStringFromHashedIndex(const char *s);

int GenerateHashwordList(char **names);

int GenerateIndexList(DWORD **index);

#endif
