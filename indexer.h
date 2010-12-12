/***************************************************************************
                          indexer.h  -  indexing support include
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef INDEXER_H_INCLUDED
#define INDEXER_H_INCLUDED

#include "basetypes.h"

// Perfomance releative params
// Higher is better, but will take more memory
#define FIRST_LEVELS_CACHE_SIZE		5

// Common unmodif. params
#define INDEXFILE_MAX_CHARTABLE 256
#define INDEXFILE_FILE_SIGNATURE "wc-idx"
#define INDEXFILE_MAX_INDEXING_WORD_LENGTH 255

/* indexer error codes */
#define INDEXFILE_ERROR_ALLOK				0
#define INDEXFILE_ERROR_NOT_INITIALIZED		1
#define INDEXFILE_ERROR_IO_ERROR			2
#define INDEXFILE_ERROR_INVALID_FILE		3
#define INDEXFILE_ERROR_INVALID_WORD		4
#define INDEXFILE_ERROR_INDEX_ALREADY_EXIST	5
#define INDEXFILE_ERROR_INDEX_NOT_EXIST		6
#define INDEXFILE_ERROR_EXIST_DIFF_INDEX	7

#define INDEXFILE_CREATE_NEW			0x0001
#define INDEXFILE_CREATE_EXISTING		0x0002
#define INDEXFILE_CREATE_BOTH			0x0004

#define INDEXFILE_NO_INDEX			0xFFFFFFFF

/* Index element structure */
struct SSymbolIndex {
	DWORD Index;
	DWORD Location;
};

/* Cache elements structure */
struct SCacheElIndex;

struct SCacheElIndex {
	SSymbolIndex *II;	// File info
	SCacheElIndex **IM;	// Memory references
};

class CIndexFile {
protected:
	WCFILE *ifh;
	WORD symcount;
	WORD symtable[INDEXFILE_MAX_CHARTABLE];
	DWORD begin_pos, maxindex;
	SSymbolIndex *iblock, *iblockFF;
	char *ifname;
	int init;

	SCacheElIndex *CacheROOT;		// ROOT element of cache block structure
	SCacheElIndex* AllocCacheBlock(DWORD symcount);
	int FreeCacheBlock(SCacheElIndex *cb);
public:
	CIndexFile(char *ifname, DWORD flags, WORD chartablelen, char *chartable, DWORD maxi);
	~CIndexFile();
	DWORD errnum;

	DWORD TestIndexWord(char *iword);
//	DWORD PutIndexByWord(char *iword, DWORD LocIndex);
	DWORD PutOrGetIndexByWordAssoc(char *iword, BYTE MinLength, void *vpms);
	DWORD GetIndexByWord(char *iword, DWORD *LocIndex, int *exact);
	DWORD ModifyIndexByWord(char *iword, DWORD oldLocIndex, DWORD NewLocIndex);
};

#endif
