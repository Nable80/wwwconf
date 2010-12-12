/***************************************************************************
                          searcher.h  -  message search support
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef SEARCHER_H_INCLUDED
#define SEARCHER_H_INCLUDED

#include "basetypes.h"
#include "indexer.h"

#define SEARCHER_INDEX_CREATE_NEW INDEXFILE_CREATE_NEW
#define SEARCHER_INDEX_CREATE_EXISTING INDEXFILE_CREATE_EXISTING
#define SEARCHER_SYMBOLS_TABLE "QWERTYUIOPASDFGHJKLZXCVBNM…÷” ≈Õ√ÿŸ«’⁄‘€¬¿œ–ŒÀƒ∆›ﬂ◊—Ã»“‹¡ﬁ®1234567890_&@!"
#define SEARCHER_MAX_CHARACTER_CODE 256
#define SEARCHER_INDEX_LENGTH	256
#define SEARCHER_MIN_WORD		3

/* searcher function error codes */
#define SEARCHER_RETURN_ALLOK				0
#define	SEARCHER_RETURN_ALREADY_EXIST		1
#define SEARCHER_RETURN_DB_ERROR			2
#define SEARCHER_RETURN_INVALID_WORD		3
#define SEARCHER_RETURN_WORD_TOO_SHORT		4
#define SEARCHER_RETURN_UNKNOWN_ERROR		5
#define SEARCHER_RETURN_WORD_NOT_FOUND		6

#define SEARCHER_NEW_CLUSTER_BLOCK_SIZE		25
#define SEARCHER_MAX_CONTIGIOUS_CLUSTERS	20

/* search cluster block header */
struct SSearcherHStruct {
	DWORD prev; // if no prev 0xFFFFFFFF
	DWORD used;
	DWORD count;
};

/* message searcher class */
class CMessageSearcher
{
protected:
	int init;
	WCFILE *f;
	int GetAllSubstrings(char *s, char **res);
public:

	DWORD errnum, indexed_word_count;
	CIndexFile *SI;
	char *symtbl;
	char *srch_str;
	DWORD srch_str_len;
	CMessageSearcher(DWORD flags);
	~CMessageSearcher();

	//int insertwordtoindex(char *word, dword mid);
	int InsertWordWithSubwordsToIndex(char *word, DWORD mid);
	int InsertMessageToIndex(char *body, DWORD mid);
	DWORD* SearchMessagesByWord(char *wmask, DWORD *count);
	DWORD* SearchMessagesByPattern(char *wpat, DWORD *count);

	DWORD AddMessageToRefBlock(DWORD bi =INDEXFILE_NO_INDEX);
	DWORD AddingMId;
};

#endif
