/***************************************************************************
                          searcher.cpp  -  message search support
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "searcher.h"

#define SEARCHER_REALLOC_ALLSUBSTRINGS_MULTI 10
#define SEARCHER_MAX_WORD_LENGTH 20

#define sr_lock_file()		{lock_file(f);}
#define sr_unlock_file()	{unlock_file(f);}
#define sr_unlock_and_db_error()	{unlock_file(f); errnum = SEARCHER_RETURN_DB_ERROR; return SEARCHER_RETURN_DB_ERROR;}
#define sr_db_error()	{errnum = SEARCHER_RETURN_DB_ERROR; return SEARCHER_RETURN_DB_ERROR;}
#define sr_db_error1()	{errnum = SEARCHER_RETURN_DB_ERROR; return NULL;}

/* toupper string using CP1251
 */
char* toupperstr(char *s)
{
	if(s == NULL) return s;
	register DWORD k = (DWORD)strlen(s);
	for(register DWORD i = 0; i < k; i++) {
		if(s[i] >= 'à' && s[i] <= 'ÿ') {
			s[i] -= ('à' - 'À');
		}
		else {
			if(!(s[i] >= 'À' && s[i] <= 'ß'))
				s[i] = (char)toupper((unsigned char)s[i]);
		}
	}
	return s;
}

/***************************
 * fast quick sort algorithm
 */
#define QSORT_MAXSTACK (sizeof(DWORD)*200)

void inline swap(DWORD *a, DWORD *b)
{
	register DWORD t=*a;
	*a=*b;
	*b = t;
}

int inline compar(const void *a, const void *b) { return *(DWORD *)a - *(DWORD *)b; }

void inline mqsort(void *base, size_t nmemb) {
    void *lbStack[QSORT_MAXSTACK], *ubStack[QSORT_MAXSTACK];
    int sp;
    DWORD offset;
    lbStack[0] = (char *)base;
    ubStack[0] = (char *)base + (nmemb-1)*sizeof(DWORD);
    for (sp = 0; sp >= 0; sp--) {
        char *lb, *ub, *m;
        char *P, *i, *j;

        lb = (char*)lbStack[sp];
        ub = (char*)ubStack[sp];

        while (lb < ub) {
            offset = DWORD((ub - lb) >> 1);
            P = lb + offset - offset % sizeof(DWORD);
            swap( (DWORD*)lb, (DWORD*)P);

            i = lb + sizeof(DWORD);
            j = ub;
            for(;;) {
				while (i < j && compar(lb, i) > 0) i += sizeof(DWORD);
                while (j >= i && compar(j, lb) > 0) j -= sizeof(DWORD);
                if (i >= j) break;
                swap((DWORD*)i, (DWORD*)j);
                j -= sizeof(DWORD);
                i += sizeof(DWORD);
            }

            swap((DWORD*)lb, (DWORD*)j);
            m = j;

            if (m - lb <= ub - m) {
                if (m + sizeof(DWORD) < ub) {
                    lbStack[sp] = m + sizeof(DWORD);
                    ubStack[sp++] = ub;
                }
                ub = m - sizeof(DWORD);
            }
			else {
                if (m - sizeof(DWORD) > lb) {
                    lbStack[sp] = lb; 
                    ubStack[sp++] = m - sizeof(DWORD);
                }
                lb = m + sizeof(DWORD);
            }
        }
    }
}

/* Linear join two sorted arrays
 * find an equals element in arrays buf1, buf2 and store them to rbuf
 * return count of the found equal elements
 */
DWORD inline join_sorted(DWORD *buf1, DWORD c1, DWORD *buf2, DWORD c2, DWORD **rbuf)
{
	DWORD c = 0, minc;
	DWORD p1 = 0, p2 = 0;
	if(c1 > c2) minc = c2; else minc = c1;
	*rbuf = (DWORD*)malloc(minc*sizeof(DWORD));
	
	for(;;) {
		if(buf1[p1] > buf2[p2]) {
			if(p2 < (c2 - 1)) p2++;
			else break;
		}
		else {
			if(buf1[p1] < buf2[p2]) {
				if(p1 < (c1 - 1)) p1++;
				else break;
			}
			else {
				(*rbuf)[c] = buf1[p1];
				c++;
				if(p2 < (c2 - 1)) p2++;
				else {
					if(p1 < (c1 - 1)) p1++;
					else break;
				}
			}
		}
	}

	if(c != 0) *rbuf = (DWORD*)realloc(*rbuf, c*sizeof(DWORD));
	else {
		free(*rbuf);
		*rbuf = NULL;
	}
	return c;
}

/***************************
 * CMessageSearcher class 
 */

/* generate all substrings of string s with length > SEARCHER_MIN_WORD
 * return number of substrings in resulting string *res, resulting string *res
 * ended with 0x00 0x00 at the end
 */
int CMessageSearcher::GetAllSubstrings(char *s, char **res)
{
	DWORD a, b, i, j, mlen, len, inc;
	DWORD wc = 0;
	b = a = (DWORD)strlen(s);
	inc = mlen = SEARCHER_REALLOC_ALLSUBSTRINGS_MULTI*b;
	*res = (char*)malloc(mlen);
	if(a <= SEARCHER_MIN_WORD) {
		strcpy(*res, s);
		*res = (char*)realloc(*res, b + 1);
		return 1;
	}
	a -= SEARCHER_MIN_WORD;
	len = 0;
	DWORD r = 0;
	for(i = 0; i <= a; i++) {
		for(j = i + SEARCHER_MIN_WORD - 1; j < b; j++) {
			char tmp = *(s + j + 1);
			*(s + j + 1) = 0;
			if(j - i + 1 +len >= mlen) {
				mlen = mlen + inc;
				*res = (char*)realloc(*res, mlen);
			}
			wc++;
			strcpy((*res) + r, s +  i);
			r += j-i+2;
			len += j-i+2;
			*(s + j + 1) = tmp;
		}
	}
	(*res)[r] = 0;
	*res = (char*)realloc(*res, len + 1);
	return wc;
}

CMessageSearcher::CMessageSearcher(DWORD flags)
{
	DWORD i;
	srch_str = NULL;
	srch_str_len = 0;
	SI = new CIndexFile(F_SEARCH_INDEX, flags, (WORD)strlen(SEARCHER_SYMBOLS_TABLE),
					SEARCHER_SYMBOLS_TABLE, SEARCHER_INDEX_LENGTH);
	f = NULL;
	symtbl = NULL;
	indexed_word_count = 0;
	if(SI->errnum != INDEXFILE_ERROR_ALLOK) {
		delete SI;
		SI = NULL;
		errnum = SEARCHER_RETURN_DB_ERROR;
		return;
	}
	if((f = wcfopen(F_SEARCH_DB,
					(flags & SEARCHER_INDEX_CREATE_NEW) ? FILE_ACCESS_MODES_CW :
					FILE_ACCESS_MODES_RW)) == NULL &&
		(f = wcfopen(F_SEARCH_DB,
					(flags & SEARCHER_INDEX_CREATE_EXISTING) ? FILE_ACCESS_MODES_CW :
					FILE_ACCESS_MODES_RW)) == NULL
	  )
	{
		delete SI;
		SI = NULL;
		errnum = SEARCHER_RETURN_DB_ERROR;
		return;
	}
	symtbl = (char*)malloc(SEARCHER_MAX_CHARACTER_CODE);
	if(symtbl == NULL) {
		delete SI;
		SI = NULL;
		errnum = SEARCHER_RETURN_DB_ERROR;
		return;
	}
	for(i = 0; i < SEARCHER_MAX_CHARACTER_CODE; i++)
		symtbl[i] = 0;

	for(i = 0; i < strlen(SEARCHER_SYMBOLS_TABLE); i++) {
		symtbl[(unsigned char)SEARCHER_SYMBOLS_TABLE[i]] = 1;
	}
	errnum = SEARCHER_RETURN_ALLOK;
}

CMessageSearcher::~CMessageSearcher()
{
	if(SI != NULL) delete SI;
	if(symtbl != NULL) free(symtbl);
	if(srch_str != NULL) free(srch_str);
	if(f != NULL) wcfclose(f);
}

/* insert word [word] with message index [mid] to search index
 * return SEARCHER_RETURN_ALLOK if successfull, otherwise error code returned
 */

//int CMessageSearcher::InsertWordToIndex(char *word, DWORD mid)
//{
//	SSearcherHStruct sb, sb1, sv_sb;
//	DWORD idx, dberr, curmalloc, pos_notfound, svpos;
//	DWORD *buf;
//
//
//	indexed_word_count++;
//
//	if((dberr = SI->GetIndexByWord(word, &idx, NULL)) == INDEXFILE_ERROR_INDEX_NOT_EXIST) {
//		/* create new index */
//		/********* lock f *********/
//		sr_lock_file();
//
//		if(wcfseek(f, 0, SEEK_END) != 0) sr_unlock_and_db_error();
//
//		idx = wcftell(f);
//
//		/* tune sb1 - new block header */
//		sb1.prev = 0xFFFFFFFF;
//		sb1.count = SEARCHER_NEW_CLUSTER_BLOCK_SIZE;
//		sb1.used = 1;
//
//		if(!fCheckedWrite(&sb1, sizeof(sb), f)) sr_unlock_and_db_error();
//		buf = (DWORD*)malloc(sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE);
//		if(buf == NULL) sr_unlock_and_db_error();
//		buf[0] = mid;
//		for(register DWORD i = 1; i < SEARCHER_NEW_CLUSTER_BLOCK_SIZE; i++) buf[i] = 0;
//		if(!fCheckedWrite(buf, sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE, f)) {
//			free(buf);
//			sr_unlock_and_db_error();
//		}
//		free(buf);
//
//		if((dberr = SI->PutIndexByWord(word, idx)) != INDEXFILE_ERROR_ALLOK)
//			sr_unlock_and_db_error();
//
//		goto Unlock_End;
//	}
//	if(dberr != INDEXFILE_ERROR_ALLOK)
//		sr_db_error();
//
//	/* add index */
//	sb.prev = idx;
//
//	if((buf = (DWORD*)malloc(sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE)) == NULL) sr_db_error();
//	curmalloc = SEARCHER_NEW_CLUSTER_BLOCK_SIZE;
//
//	/********* lock f *********/
//	sr_lock_file();
//
//	pos_notfound = 1;
//
//	if(sb.prev != 0xFFFFFFFF){
//		if(wcfseek(f, sb.Next, SEEK_SET) != 0) sr_unlock_and_db_error();
//
//		idx = sb.prev;
//
//		if(!fCheckedRead(&sb, sizeof(SSearcherHStruct), f))
//			sr_unlock_and_db_error();
//
//		/* is block to long ? */
//		if(sb.used > curmalloc) {
//			buf = (DWORD*)realloc(buf, sb.used*sizeof(DWORD));
//			curmalloc = sb.count;
//		}
//
//		if(!fCheckedRead(buf, sizeof(DWORD)*sb.used, f)) sr_unlock_and_db_error();
//
//		for(register DWORD i = 0; i < sb.used; i++) {
//			if(buf[i] == mid) {
//				/* already exist ! shouldn't add again */
//				free(buf);
//				goto Unlock_End;
//			}
//		}
//
//		if(sb.used < sb.count && pos_notfound) {
//			/* save first usefull block */
//			pos_notfound = 0;
//			sv_sb = sb;
//			svpos = idx;
//		}
//	}
//
//	free(buf);
//
//	if(pos_notfound == 1) {
//		/* add new block */
//		if(wcfseek(f, 0, SEEK_END) != 0) sr_unlock_and_db_error();
//		sb.Next = wcftell(f);
//
//		/* tune sb1 - new block header */
//		sb1.Next = 0xFFFFFFFF;
//		sb1.count = SEARCHER_NEW_CLUSTER_BLOCK_SIZE;
//		sb1.used = 1;
//
//		/* save new cluster block */
//		if(!fCheckedWrite(&sb1, sizeof(sb), f)) sr_unlock_and_db_error();
//		buf = (DWORD*)malloc(sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE);
//		if(buf == NULL) sr_unlock_and_db_error();
//		buf[0] = mid;
//		for(register DWORD i = 1; i < SEARCHER_NEW_CLUSTER_BLOCK_SIZE; i++) buf[i] = 0;
//		if(!fCheckedWrite(buf, sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE, f)) {
//			free(buf);
//			sr_unlock_and_db_error();
//		}
//		free(buf);
//		if(wcfseek(f, idx, SEEK_SET) != 0) sr_unlock_and_db_error();
//		if(!fCheckedWrite(&sb, sizeof(sb), f)) sr_unlock_and_db_error();
//	}
//	else {
//		/* place for add found */
//		sv_sb.used++;
//
//		if(wcfseek(f, svpos, SEEK_SET) != 0) sr_unlock_and_db_error();
//
//		if(!fCheckedWrite(&sv_sb, sizeof(sv_sb), f)) sr_unlock_and_db_error();
//
//		if(wcfseek(f, svpos + sizeof(sv_sb) + (sv_sb.used - 1)*sizeof(mid), SEEK_SET) != 0)
//			sr_unlock_and_db_error();
//		if(!fCheckedWrite(&mid, sizeof(mid), f))  sr_unlock_and_db_error();
//	}
//
//Unlock_End:
//	sr_unlock_file();
//	/****** unlock f ******/
//
//	errnum = SEARCHER_RETURN_ALLOK;
//	return SEARCHER_RETURN_ALLOK;
//}

/*
 * return 1 if successfull otherwise zero returned
 */
/* Function create new block for storing references to messages(blocks, clusters, etc.) if bi = 0xFFFFFFF
 * and put to new block message with CMessageSearcher::AddingMId
 * return Id of created block or 0xFFFFFFFF if an error occured
 */
/*DWORD CMessageSearcher::AllocRefBlock()*/
/*DWORD CMessageSearcher::AddMessageToRefBlock(DWORD bi)*/
/*  changed, get last block, checking last MId, adding, retunrnig block address   */


DWORD CMessageSearcher::AddMessageToRefBlock(DWORD bi)
{
	SSearcherHStruct sb;
	DWORD buf[SEARCHER_NEW_CLUSTER_BLOCK_SIZE*SEARCHER_MAX_CONTIGIOUS_CLUSTERS];
	bool add_new = 1;
	// we just have found out that there is an existing block for this word
	// so we should only add new message index

	if (bi != INDEXFILE_NO_INDEX){
		if(wcfseek(f, bi, SEEK_SET) != 0) goto Ret_Error;

		if(!fCheckedRead(&sb, sizeof(SSearcherHStruct), f))
			goto Ret_Error;

		// is block to long ?
		if(sb.used > SEARCHER_NEW_CLUSTER_BLOCK_SIZE*SEARCHER_MAX_CONTIGIOUS_CLUSTERS) {
			// ASSERTION HERE - we do not support such large blocks
			goto Ret_Error;
		}

		if(!fCheckedRead(buf, sizeof(DWORD)*sb.used, f))
			goto Ret_Error;

		// check last id
		// AlexBi: If we are having full block here it's not bad idea to check through it
		for(register DWORD i = 0; i < sb.used; i++) {
			if(buf[i] == AddingMId) {
				// already exist ! shouldn't add again
				goto Ret_Ok;
			}
		}

		if(sb.used < sb.count) {
			// save first usefull block
			sb.used++;
			// update block header
			if(wcfseek(f, bi, SEEK_SET) != 0) goto Ret_Error;
			if(!fCheckedWrite(&sb, sizeof(sb), f)) goto Ret_Error;
	
			// update block data
			if(wcfseek(f, bi + sizeof(sb) + (sb.used - 1)*sizeof(AddingMId), SEEK_SET) != 0)
				goto Ret_Error;
			if(!fCheckedWrite(&AddingMId, sizeof(AddingMId), f))  goto Ret_Error;
			add_new = 0;
		}
	}
	if(add_new) {
		// add new block
	
		/* tune sb - new block header */
		sb.prev = bi;
		sb.count = SEARCHER_NEW_CLUSTER_BLOCK_SIZE;
		sb.used = 1;
		
		//get place
		if(wcfseek(f, 0, SEEK_END) != 0) goto Ret_Error;
		bi = wcftell(f);

		// save new block header
		if(!fCheckedWrite(&sb, sizeof(sb), f)) goto Ret_Error;

		// create new block data
		memset((void*)(&buf), 0, sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE);
		buf[0] = AddingMId;
		// save new block data
		if(!fCheckedWrite(buf, sizeof(DWORD)*SEARCHER_NEW_CLUSTER_BLOCK_SIZE, f)) {
			goto Ret_Error;
		}
	}
Ret_Ok:
	//return current last block 
	return bi;
Ret_Error:
	return 0;
}

/* insert word [word] and subwords begining with [word] (lenght from 
 * SEARCHER_MIN_WORD to [word]) with message index [mid] to search index
 * return SEARCHER_RETURN_ALLOK if successfull, otherwise error code returned
 */
int CMessageSearcher::InsertWordWithSubwordsToIndex(char *word, DWORD mid)
{
	indexed_word_count++;

	// create new index 
	//********* lock f **********/
	sr_lock_file();

	// We have already allocated a new block if we'll need it later, so continue adding
	// the word. Later, if we find out that do not need this block we simply skip creation step.
	// this is not optimal realization, and later we should use a procedure varables.
	// Then we should call PutOrGetIndexByWordAssoc()

	AddingMId = mid;
	if(SI->PutOrGetIndexByWordAssoc(word, SEARCHER_MIN_WORD - 1, this) != INDEXFILE_ERROR_ALLOK)
		sr_unlock_and_db_error();

	sr_unlock_file();
	/****** unlock f ******/
	
	errnum = SEARCHER_RETURN_ALLOK;
	return SEARCHER_RETURN_ALLOK;
}


/* insert message with VIndex = mid to index
 * return SEARCHER_RETURN_ALLOK if successfull, otherwise error code returned
 */
int CMessageSearcher::InsertMessageToIndex(char *body, DWORD mid)
{
	DWORD i, rr;
	toupperstr(body);
	do {
		i = 0;
		while(body[i] != 0 && symtbl[(unsigned char)body[i]] == 1 && i < SEARCHER_MAX_WORD_LENGTH) i++;
		if(i >= SEARCHER_MIN_WORD) {
			char x = body[i];
			body[i] = 0;

			DWORD a = i - SEARCHER_MIN_WORD;
			for(register DWORD k = 0; k <= a; k++) {
				rr = InsertWordWithSubwordsToIndex(body + k, mid);
				if(rr != SEARCHER_RETURN_ALLOK) return rr;
			}

			/*DWORD *xx;
			DWORD cnt;
			if((xx = SearchMessagesByPattern(body + 1, &cnt)) != 0) {
				free(xx);
			}*/

			body[i] = x;
		}
		if(body[i] != 0) body += i + 1;
		else body += i;

	} while(*body != 0);
	errnum = SEARCHER_RETURN_ALLOK;
	return SEARCHER_RETURN_ALLOK;
}

DWORD* CMessageSearcher::SearchMessagesByWord(char *wmask, DWORD *count)
{
	DWORD cd;
	int uniq;
	SSearcherHStruct sb;
	DWORD *buf;

	*count = 0;

	if((cd = SI->GetIndexByWord(wmask, &(sb.prev), &uniq)) != INDEXFILE_ERROR_ALLOK) {
		switch(cd) {
		case INDEXFILE_ERROR_INVALID_WORD:
			errnum = SEARCHER_RETURN_INVALID_WORD;
			return NULL;
			break;
		case INDEXFILE_ERROR_INDEX_NOT_EXIST:
			errnum = SEARCHER_RETURN_WORD_NOT_FOUND;
			return NULL;
			break;
		default:
			errnum = SEARCHER_RETURN_DB_ERROR;
			return NULL;
		}
	}

	buf = (DWORD*)malloc(1);
	while(sb.prev != 0xFFFFFFFF) {
		if(wcfseek(f, sb.prev, SEEK_SET) != 0) {
			free(buf);
			sr_db_error1();
		}
		if(!fCheckedRead(&sb, sizeof(sb), f)) {
			free(buf);
			sr_db_error1();
		}
		
		buf = (DWORD*)realloc(buf, (*count)*sizeof(DWORD) +  sb.used*sizeof(DWORD) + 1);

		if(!fCheckedRead(&(buf[*count]), sb.used*sizeof(DWORD), f)) {
			free(buf);
			sr_db_error1();
		}
		*count += sb.used;
	}
	errnum = SEARCHER_RETURN_ALLOK;
	return buf;
}

DWORD* CMessageSearcher::SearchMessagesByPattern(char *wpat, DWORD *count)
{
	DWORD *buf, *res;
	DWORD i, cnt, rescnt = 0;
	*count = 0;
	res = NULL;
	
	toupperstr(wpat);

	if(srch_str == NULL) {
		srch_str = (char*)malloc(1);
		srch_str_len = 1;
	}
	srch_str[0] = 0;

	do {
		i = 0;
		while(wpat[i] != 0 && symtbl[(unsigned char)wpat[i]] == 1) i++;
		if(i >= SEARCHER_MIN_WORD) {
			char x = wpat[i];
			wpat[i] = 0;
			if(srch_str_len <= (strlen(srch_str) + strlen(wpat) + 10)) {
				srch_str = (char*)realloc(srch_str, (strlen(srch_str) + strlen(wpat) + 10));
				srch_str_len = (DWORD)strlen(srch_str) + strlen(wpat) + 10;
				/* first search ? */
				if(res != NULL) {
					strcat(srch_str, " & ");
				}
				else strcat(srch_str, " ");
				strcat(srch_str, wpat);
			}
			buf = SearchMessagesByWord(wpat, &cnt);
			wpat[i] = x;
			if(errnum == SEARCHER_RETURN_ALLOK && buf == NULL)
				errnum = SEARCHER_RETURN_WORD_NOT_FOUND;
			switch(errnum) {

			case SEARCHER_RETURN_WORD_NOT_FOUND:
				if(res != NULL) free(res);
				res = NULL;
				*count = 0;
				goto Finishing;

			case SEARCHER_RETURN_ALLOK:
				mqsort(buf, cnt);
				if(res != NULL) {
					DWORD *buf1;
					rescnt = join_sorted(res, rescnt, buf, cnt, &buf1);
					free(buf);
					free(res);
					res = buf1;
				}
				else {
					res = buf;
					rescnt = cnt;
				}
				break;

			default:
				errnum = SEARCHER_RETURN_DB_ERROR;
				return NULL;
			}
		}
		if(wpat[i] != 0) wpat += i + 1;
		else wpat += i;
	} while(*wpat != 0);
	*count = rescnt;

Finishing:
	srch_str_len = (DWORD)strlen(srch_str) + 1;
	srch_str = (char*)realloc(srch_str, strlen(srch_str) + 1);
	errnum = SEARCHER_RETURN_ALLOK;
	return res;
}
