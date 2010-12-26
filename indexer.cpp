/***************************************************************************
                          indexer.cpp  -  indexing support
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "indexer.h"
#include "searcher.h"

#define INDEXER_RUNTIME_CHECK        0

#define unlock_and_indexer_io_error0() {unlock_file(ifh);errnum = INDEXFILE_ERROR_IO_ERROR;return;}
#define unlock_and_indexer_io_error1() {unlock_file(ifh);errnum = INDEXFILE_ERROR_IO_ERROR;return INDEXFILE_ERROR_IO_ERROR;}
#define indexer_io_error0() {errnum = INDEXFILE_ERROR_IO_ERROR; return;}
#define indexer_io_error1() {errnum = INDEXFILE_ERROR_IO_ERROR; return INDEXFILE_ERROR_IO_ERROR;}
#define indexer_lock_file() {lock_file(ifh);}
#define indexer_unlock_file() {unlock_file(ifh);}

/* Indexer class constructor: open or create
 * chartable must be NULL if CREATE_NEW not set 
 */
CIndexFile::CIndexFile(const char *ifname, DWORD flags, WORD chartablelen, const char *chartable, DWORD maxi)
{
        init = 0;
        DWORD i;

        iblock = (SSymbolIndex *)malloc(sizeof(SSymbolIndex)*chartablelen);

        if((flags & INDEXFILE_CREATE_BOTH) != 0)
        {
                /* open existing index file */
                if((ifh = wcfopen(ifname, FILE_ACCESS_MODES_RW)) != NULL)
                {
                        char *sign = (char*)malloc(strlen(INDEXFILE_FILE_SIGNATURE) + 1);
                        if(wcfread(sign, 1, strlen(INDEXFILE_FILE_SIGNATURE), ifh) !=
                                strlen(INDEXFILE_FILE_SIGNATURE))
                        {
                                free(sign);
                                flags = INDEXFILE_CREATE_NEW;
                        }
                        if(strncmp(sign, INDEXFILE_FILE_SIGNATURE, strlen(INDEXFILE_FILE_SIGNATURE)) != 0)
                        {
                                errnum = INDEXFILE_ERROR_INVALID_FILE;
                                free(sign);
                                flags = INDEXFILE_CREATE_NEW;
                        }
                        free(sign);
                        flags = INDEXFILE_CREATE_EXISTING;
                }
                else
                {
                        flags = INDEXFILE_CREATE_NEW;
                }
        }

        if((flags & INDEXFILE_CREATE_NEW) != 0)
        {
                /* create new index file */
                if((ifh = wcfopen(ifname, FILE_ACCESS_MODES_CW)) != NULL)
                {
                        indexer_lock_file();
                        /* wrtie signature */
                        if(wcfwrite(INDEXFILE_FILE_SIGNATURE, 1, strlen(INDEXFILE_FILE_SIGNATURE), ifh) !=
                                strlen(INDEXFILE_FILE_SIGNATURE))
                        {
                                free(iblock);
                                unlock_and_indexer_io_error0();
                        }
                        /*  write max indexing length (maxi) */
                        if(wcfwrite(&maxi, 1, sizeof(maxi), ifh) != sizeof(maxi))
                        {
                                free(iblock);
                                unlock_and_indexer_io_error0();
                        }
                        /* write symbol count and table */
                        if(wcfwrite(&chartablelen, 1, sizeof(chartablelen), ifh) != sizeof(chartablelen))
                        {
                                free(iblock);
                                unlock_and_indexer_io_error0();
                        }
                        if(wcfwrite(chartable, 1, chartablelen, ifh) != chartablelen)
                        {
                                free(iblock);
                                unlock_and_indexer_io_error0();
                        }

                        memset(iblock, 0xFF, chartablelen*sizeof(SSymbolIndex));
                        if(wcfwrite(iblock, 1, chartablelen*sizeof(SSymbolIndex), ifh) != chartablelen*sizeof(SSymbolIndex))
                        {
                                free(iblock);
                                unlock_and_indexer_io_error0();
                        }

                        indexer_unlock_file();
                        wcfclose(ifh);
                }
                else indexer_io_error0();
                flags = INDEXFILE_CREATE_EXISTING;
        }

        if((flags & INDEXFILE_CREATE_EXISTING) != 0)
        {
                /* open existing index file */
                if((ifh = wcfopen(ifname, FILE_ACCESS_MODES_RW)) != NULL)
                {
                        char *sign = (char*)malloc(strlen(INDEXFILE_FILE_SIGNATURE) + 1);
                        if(wcfread(sign, 1, strlen(INDEXFILE_FILE_SIGNATURE), ifh) !=
                                strlen(INDEXFILE_FILE_SIGNATURE))
                        {
                                free(sign);
                                free(iblock);
                                indexer_io_error0();
                        }
                        if(strncmp(sign, INDEXFILE_FILE_SIGNATURE, strlen(INDEXFILE_FILE_SIGNATURE)) != 0)
                        {
                                errnum = INDEXFILE_ERROR_INVALID_FILE;
                                free(sign);
                                free(iblock);
                                return;
                        }
                        free(sign);

                        /* read maximum index length */
                        if(wcfread(&maxindex, 1, sizeof(maxindex), ifh) != sizeof(maxindex))
                        {
                                free(iblock);
                                indexer_io_error0();
                        }
                        /* read symbolic count */
                        if(wcfread(&symcount, 1, sizeof(symcount), ifh) != sizeof(symcount))
                        {
                                free(iblock);
                                indexer_io_error0();
                        }
                        // realloc iblock for new symcount
                        iblock = (SSymbolIndex *)realloc(iblock, sizeof(SSymbolIndex)*symcount);

                        char *chartbl = (char*)malloc(symcount + 10);
                        if(wcfread(chartbl, 1, symcount, ifh) != symcount)
                        {
                                free(chartbl);
                                indexer_io_error0();
                        }
                        /* prepare index of the begining */
                        begin_pos = wcftell(ifh);
                        
                        if(wcfread(iblock, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
                        {
                                free(iblock);
                                free(chartbl);
                                indexer_io_error0()
                        }
                        // prepare symtable indexes
                        for(i = 0; i < INDEXFILE_MAX_CHARTABLE; i++) symtable[i] = INDEXFILE_MAX_CHARTABLE;
                        for(i = 0; i < symcount; i++)
                                symtable[(unsigned char)chartbl[i]] = (unsigned char)i;
                        free(chartbl);
                        // Activate cache
                        CacheROOT = AllocCacheBlock(symcount);
                        memcpy(CacheROOT->II, iblock, sizeof(SSymbolIndex)*symcount);
                }
                else {
                        free(iblock);
                        indexer_io_error0();
                }

                init = 1;
                errnum = INDEXFILE_ERROR_ALLOK;
                // save file name
                CIndexFile::ifname = (char*)malloc(strlen(ifname) + 1);
                strcpy(CIndexFile::ifname, ifname);
                // make empty block
                iblockFF = (SSymbolIndex*)malloc(sizeof(SSymbolIndex)*symcount);
                memset(iblockFF, 0xFF, sizeof(SSymbolIndex)*symcount);
                return;
        }
}

CIndexFile::~CIndexFile()
{
        if(init)
        {
                DWORD lev = 0;
                DWORD j[FIRST_LEVELS_CACHE_SIZE];
                int recursion = 0;
                DWORD begfrom = 0;
                SCacheElIndex *tcb[FIRST_LEVELS_CACHE_SIZE];
                tcb[0] = CacheROOT;
                if(wcfseek(ifh, begin_pos, SEEK_SET) != 0)
                {
                        indexer_io_error0();
                }
                if(wcfwrite(tcb[lev]->II, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex)) 
                {
                        indexer_io_error0()
                }

                for(;;)
                {

                        for(j[lev] = begfrom; j[lev] < symcount; j[lev]++)
                        {
                                if((DWORD)tcb[lev]->IM[j[lev]] != INDEXFILE_NO_INDEX)
                                {
                                        if(wcfseek(ifh, tcb[lev]->II[j[lev]].Index, SEEK_SET) != 0)
                                        {
                                                indexer_io_error0();
                                        }
                                        if(wcfwrite(tcb[lev]->IM[j[lev]]->II, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
                                        {
                                                indexer_io_error0()
                                        }
                                        lev++;
                                        tcb[lev] = tcb[lev-1]->IM[j[lev-1]];
                                        recursion = 1;
                                        break;
                                }
                        }
                        if(!recursion)
                        {
                                FreeCacheBlock(tcb[lev]);
                                if(lev) lev--; 
                                else break; // go out at all
                                j[lev]++;
                                begfrom = j[lev];
                        }
                        else
                        {
                                begfrom = 0;
                                recursion = 0;
                        }
                }

                free(ifname);
                free(iblock);
                free(iblockFF);
                wcfclose(ifh);
        }
}


/* Allocate cache block for defined symcount
 * if successfull returns pointer to SCacheElIndex, otherwise returns NULL
 */
SCacheElIndex* CIndexFile::AllocCacheBlock(DWORD symcount)
{
        SCacheElIndex *p;
        p = (SCacheElIndex*)malloc(sizeof(SCacheElIndex));
        if(!p) return p;
        p->II = new SSymbolIndex[symcount];
        if(!p->II)
        {
                free(p);
                return NULL;
        }
        memset(p->II, 0xFF, sizeof(SSymbolIndex)*symcount);
        p->IM = (SCacheElIndex**)malloc(sizeof(SCacheElIndex*)*symcount);
        if(!p->IM)
        {
                free(p->II);
                free(p);
                return NULL;
        }
        memset(p->IM, 0xFF, sizeof(SCacheElIndex*)*symcount);
        return p;
}


/* Free cache block
 */
int CIndexFile::FreeCacheBlock(SCacheElIndex *cb)
{
        free(cb->II);
        free(cb->IM);
        free(cb);
        return 1;
}


/* Test if word [iword] is compartible with current index file
 * return 1 if compartible, otherwise 0
 */
DWORD CIndexFile::TestIndexWord(char *iword)
{
        for(DWORD i = 0; i < strlen(iword); i++)
        {
                if(((unsigned char)iword[i]) > INDEXFILE_MAX_CHARTABLE || symtable[(unsigned char)iword[i]] == INDEXFILE_MAX_CHARTABLE)
                {
                        return 0;
                }
        }
        return 1;
}

/* put new index entry for word iword to indexes,
 * if index already exist it return INDEXFILE_ERROR_INDEX_ALREADY_EXIST
 */
//DWORD CIndexFile::PutIndexByWord(char *iword, DWORD LocIndex)
//{
//        DWORD indlength, curpos, cpos, i;
//        SCacheElIndex *bcache;
//        SSymbolIndex *iblk;
//
//#if INDEXER_RUNTIME_CHECK
//        if(ifh == NULL)
//        {
//                errnum = INDEXFILE_ERROR_INVALID_FILE;
//                return errnum;
//        }
//        if(TestIndexWord(iword) == 0)
//        {
//                errnum = INDEXFILE_ERROR_INVALID_WORD;
//                return errnum;
//        }
//#endif
//
//        DWORD strlen_iword = (DWORD)strlen(iword);
//
//        if(strlen_iword != 0 && strlen_iword <= INDEXFILE_MAX_INDEXING_WORD_LENGTH)
//        {
//                curpos = begin_pos;
//                indexer_lock_file();
//
//                // prepare cache
//                bcache = CacheROOT;
//
//                if(strlen_iword > maxindex) indlength = maxindex;
//                else indlength = strlen_iword;
//
//                for(i = 0; i < indlength; i++)
//                {
//                        if(i < FIRST_LEVELS_CACHE_SIZE)
//                        {
//                                if(i != 0)
//                                {
//                                        bcache = bcache->IM[cpos];
//                                }
//                                iblk = bcache->II;
//                        }
//                        else
//                        {
//                                iblk = iblock;
//                                if(wcfseek(ifh, curpos, SEEK_SET) != 0)
//                                {
//                                        unlock_and_indexer_io_error1();
//                                }
//                                if(wcfread(iblk, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
//                                {
//                                        unlock_and_indexer_io_error1();
//                                }
//                        }
//
//                        cpos = symtable[(unsigned char)iword[i]];
//
//                        if(iblk[cpos].Index == INDEXFILE_NO_INDEX && i != indlength - 1)
//                        {
//                                // create new block - we should write it to disk PERMANENTLY !
//
//                                if(wcfseek(ifh, 0, SEEK_END) != 0)
//                                {
//                                        unlock_and_indexer_io_error1();
//                                }
//                                DWORD fpos = wcftell(ifh);
//                                if(wcfwrite(iblockFF, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
//                                {
//                                        unlock_and_indexer_io_error1();
//                                }
//
//                                // now we decide what we should do: write new index to cache or to disk
//                                // and where we will allocate new block - in cache or on drive
//                                if(i < FIRST_LEVELS_CACHE_SIZE)
//                                {
//                                        if(i < FIRST_LEVELS_CACHE_SIZE - 1)
//                                        {
//                                                // write to cache and allocate new in cache
//                                                bcache->IM[cpos] = AllocCacheBlock(symcount);
//                                                // write new index to cache
//                                                bcache->II[cpos].Index = fpos;
//                                        }
//                                        else
//                                        {
//                                                // write to cache but allocate new on disk
//                                                bcache->II[cpos].Index = fpos;
//                                        }
//                                        curpos = bcache->II[cpos].Index;
//                                }
//                                else
//                                {
//                                        // write to disk and allocate new on disk
//                                        iblk[cpos].Index = fpos;
//                                        if(wcfseek(ifh, curpos, SEEK_SET) != 0)
//                                        {
//                                                unlock_and_indexer_io_error1();
//                                        }
//                                        if(wcfwrite(iblk, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
//                                        {
//                                                unlock_and_indexer_io_error1();
//                                        }
//                                        curpos = iblk[cpos].Index;
//                                }
//                        }
//                        else
//                        {
//                                if(i != indlength - 1)
//                                {
//                                        curpos = iblk[cpos].Index;
//                                        if(i < FIRST_LEVELS_CACHE_SIZE - 1 && (DWORD)bcache->IM[cpos] == INDEXFILE_NO_INDEX)
//                                        {
//                                                // load to cache, if we need it
//
//                                                // allocate block
//                                                bcache->IM[cpos] = AllocCacheBlock(symcount);
//
//                                                // and read index from disk
//                                                if(wcfseek(ifh, iblk[cpos].Index, SEEK_SET) != 0)
//                                                {
//                                                        unlock_and_indexer_io_error1();
//                                                }
//                                                if(wcfread(bcache->IM[cpos]->II, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
//                                                {
//                                                        unlock_and_indexer_io_error1();
//                                                }
//                                        }
//                                }
//                        }//of else
//                }
//
//                if(iblk[cpos].Location != INDEXFILE_NO_INDEX)
//                {
//                        indexer_unlock_file();
//                        errnum = INDEXFILE_ERROR_INDEX_ALREADY_EXIST;
//                        return errnum;
//                }
//                else {
//                        // save index
//                        iblk[cpos].Location = LocIndex;
//                        if(indlength > FIRST_LEVELS_CACHE_SIZE)
//                        {
//                                // write to drive
//                                if(wcfseek(ifh, curpos, SEEK_SET) != 0)
//                                {
//                                        unlock_and_indexer_io_error1();
//                                }
//                                DWORD rr = (DWORD)wcfwrite(iblk, 1, symcount*sizeof(SSymbolIndex), ifh);
//                                indexer_unlock_file();
//                                if(rr != symcount*sizeof(SSymbolIndex))
//                                {
//                                        indexer_io_error1();
//                                }
//                        }
//                }
//        }
//        else
//        {
//                errnum = INDEXFILE_ERROR_INVALID_WORD;
//                return errnum;
//        }
//        errnum = INDEXFILE_ERROR_ALLOK;
//        return errnum;
//}


/* put new index entry for word iword to indexes,
 * if index is not the same for subwords functions aborts and return
 * INDEXFILE_ERROR_EXIST_DIFF_INDEX
 * BEFORE CALLING THIS FUNCTION : Should be set AddingMId at CMessageSearcher because of 
 * this function using calls to CMessageSearcher::AddMessageToRefBlock() and AllocRefBlock()
 ***********************************************************************
 * THERE IS SOME ADMISSION : IF there is an index for word with lenght n 
 * => we should have also index for all words between MinLenght and default 
 * word length. If say other it means that if you are using this functions you can't
 * in addition to use PutIndexByWord, because of database corruption will take place.
 */
DWORD CIndexFile::PutOrGetIndexByWordAssoc(char *iword, BYTE MinLength, void *vpms)// DWORD NewBlockMark, DWORD NewBlockDelta)
{
        CMessageSearcher *pMS = (CMessageSearcher*)vpms;
        DWORD indlength, curpos, cpos = 0, NewLocation;
        DWORD i;
        SCacheElIndex *bcache;
        SSymbolIndex *iblk;

#if INDEXER_RUNTIME_CHECK
        if(ifh == NULL)
        {
                errnum = INDEXFILE_ERROR_INVALID_FILE;
                return errnum;
        }
        if(TestIndexWord(iword) == 0)
        {
                errnum = INDEXFILE_ERROR_INVALID_WORD;
                return errnum;
        }
#endif

        DWORD strlen_iword = (DWORD)strlen(iword);

        if(strlen_iword != 0 && strlen_iword <= INDEXFILE_MAX_INDEXING_WORD_LENGTH)
        {
                curpos = begin_pos;
                indexer_lock_file();

                // prepare cache
                bcache = CacheROOT;

                if(strlen_iword > maxindex) indlength = maxindex;
                else indlength = strlen_iword;

                for(i = 0; i < indlength; i++)
                {
                        if(i < FIRST_LEVELS_CACHE_SIZE)
                        {
                                if(i != 0)
                                {
                                        bcache = bcache->IM[cpos];
                                }
                                iblk = bcache->II;
                        }
                        else
                        {
                                iblk = iblock;
                                if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                                if(wcfread(iblk, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                        }

                        cpos = symtable[(unsigned char)iword[i]];

                        // add new id to block, update location if new block is created
                        if(i >= MinLength)
                        {
                                NewLocation = pMS->AddMessageToRefBlock(iblk[cpos].Location);
                                if(NewLocation != iblk[cpos].Location && i >= FIRST_LEVELS_CACHE_SIZE) {
                                        iblk[cpos].Location = NewLocation;
                                        // update with new index only in case we are missing cache =)
                                        if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                                        {
                                                unlock_and_indexer_io_error1();
                                        }
                                        if(wcfwrite(iblk, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
                                        {
                                                unlock_and_indexer_io_error1();
                                        }
                                }
                                else iblk[cpos].Location = NewLocation;
                        }

                        // go else
                        if(iblk[cpos].Index == INDEXFILE_NO_INDEX && i != indlength - 1)
                        {
                                // create new block - we should write it to disk PERMANENTLY !

                                if(wcfseek(ifh, 0, SEEK_END) != 0)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                                DWORD fpos = wcftell(ifh);
                                if(wcfwrite(iblockFF, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
                                {
                                        unlock_and_indexer_io_error1();
                                }

                                // now we decide what we should do: write new index to cache or to disk
                                // and where we will allocate new block - in cache or on drive
                                if(i < FIRST_LEVELS_CACHE_SIZE)
                                {
                                        // write new index to cache
                                        bcache->II[cpos].Index = fpos;
                                        if(i < FIRST_LEVELS_CACHE_SIZE - 1)
                                        {
                                                // write to cache and allocate new in cache
                                                bcache->IM[cpos] = AllocCacheBlock(symcount);
                                        }
                                        curpos = bcache->II[cpos].Index;
                                }
                                else
                                {
                                        // write to disk and allocate new on disk
                                        iblk[cpos].Index = fpos;
                                        if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                                        {
                                                unlock_and_indexer_io_error1();
                                        }
                                        if(wcfwrite(iblk, 1, symcount*sizeof(SSymbolIndex), ifh) != symcount*sizeof(SSymbolIndex))
                                        {
                                                unlock_and_indexer_io_error1();
                                        }
                                        curpos = iblk[cpos].Index;
                                }
                        }
                        else
                        {
                                if(i != indlength - 1)
                                {
                                        curpos = iblk[cpos].Index;
                                        if(i < FIRST_LEVELS_CACHE_SIZE - 1 && (DWORD)bcache->IM[cpos] == INDEXFILE_NO_INDEX)
                                        {
                                                // load to cache, if we need it

                                                // allocate block
                                                bcache->IM[cpos] = AllocCacheBlock(symcount);

                                                // and read index from disk
                                                if(wcfseek(ifh, iblk[cpos].Index, SEEK_SET) != 0)
                                                {
                                                        unlock_and_indexer_io_error1();
                                                }
                                                if(wcfread(bcache->IM[cpos]->II, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
                                                {
                                                        unlock_and_indexer_io_error1();
                                                }
                                        }
                                }
                        }//of else
                }
                // condition 
                // iblk[cpos].Location != INDEXFILE_NO_INDEX
                // will be true always

                if(indlength > FIRST_LEVELS_CACHE_SIZE)
                {
                        // save index
                        if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                        {
                                unlock_and_indexer_io_error1();
                        }
                        DWORD rr = (DWORD)wcfwrite(iblk, 1, symcount*sizeof(SSymbolIndex), ifh);
                        if(rr != symcount*sizeof(SSymbolIndex))
                        {
                                unlock_and_indexer_io_error1();
                        }
                }
                indexer_unlock_file();
                errnum = INDEXFILE_ERROR_ALLOK;
                return errnum;
        }
        else
        {
                errnum = INDEXFILE_ERROR_INVALID_WORD;
                return errnum;
        }
}


/* get index entry for word [iword]
 * return values: if index exist return INDEXFILE_ALLOK and if LocIndex not NULL
 * store here returning index, and also if it is uniq index  and exact not NULL 
 * set exact set it = 1, otherwise = 0, 
 * if index not exist return INDEXFILE_INDEX_NOT_EXIST
 */
DWORD CIndexFile::GetIndexByWord(char *iword, DWORD *LocIndex, int *exact)
{
        DWORD curpos, i, cpos = 0;
        DWORD indlength;
        SCacheElIndex *bcache;
        SSymbolIndex *iblk = NULL;

#if INDEXER_RUNTIME_CHECK
        if(ifh == NULL || iblock == NULL)
        {
                errnum = INDEXFILE_ERROR_INVALID_FILE;
                return errnum;
        }
        if(TestIndexWord(iword) == 0)
        {
                errnum = INDEXFILE_ERROR_INVALID_WORD;
                return errnum;
        }
#endif

        DWORD strlen_iword = (DWORD)strlen(iword); // just for optimizing

        if(strlen_iword > 1)
        {
                curpos = begin_pos;

                // prepare cache
                bcache = CacheROOT;

                if(strlen_iword > maxindex) indlength = maxindex;
                else indlength = strlen_iword;

                for(i = 0; i < indlength; i++)
                {

                        if(i < FIRST_LEVELS_CACHE_SIZE)
                        {
                                if(i != 0)
                                {
                                        if((DWORD)bcache->IM[cpos] == INDEXFILE_NO_INDEX)
                                        {
                                                // load to cache

                                                // allocate block
                                                bcache->IM[cpos] = AllocCacheBlock(symcount);

                                                // and read index from disk
                                                if(wcfseek(ifh, iblk[cpos].Index, SEEK_SET) != 0)
                                                {
                                                        unlock_and_indexer_io_error1();
                                                }
                                                if(wcfread(bcache->IM[cpos]->II, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
                                                {
                                                        unlock_and_indexer_io_error1();
                                                }

                                        }
                                        // get it from cache
                                        bcache = bcache->IM[cpos];
                                }
                                iblk = bcache->II;
                        }
                        else
                        {
                                iblk = iblock;
                                if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                                if(wcfread(iblk, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                        }

                        cpos = symtable[(unsigned char)iword[i]];

                        if(i != indlength - 1)
                        {
                                if(iblk[cpos].Index == INDEXFILE_NO_INDEX)
                                {
                                        errnum = INDEXFILE_ERROR_INDEX_NOT_EXIST;
                                        return errnum;
                                }
                                else
                                {
                                        curpos = iblk[cpos].Index;
                                }
                        }
                }

                if(LocIndex != NULL) *LocIndex = iblk[cpos].Location;
                if(exact != NULL)
                {
                        if(strlen_iword <= maxindex) *exact = 1;
                        else *exact = 0;
                }
                errnum = INDEXFILE_ERROR_ALLOK;
                return errnum;
        }
        else
        {
                errnum = INDEXFILE_ERROR_INVALID_WORD;
                return errnum;
        }
}


/* NEW VERSION DOESN'T TESTED AT ALL !!! */

/* modify index associated with word [iword] to NewLocIndex if oldLocIndex equal 
 * modifying index otherwise return error code (INDEXFILE_ERROR_INDEX_NOT_EXIST 
 * returned in that case if oldLocIndex != index of modifying word)
 * (if it need to delete index - NewLocIndex should be set to INDEXFILE_NO_INDEX)
 */
DWORD CIndexFile::ModifyIndexByWord(char *iword, DWORD oldLocIndex, DWORD NewLocIndex)
{
        DWORD curpos, i, cpos = 0;
        DWORD indlength;
        SCacheElIndex *bcache;
        SSymbolIndex *iblk = NULL;

#if INDEXER_RUNTIME_CHECK
        if(ifh == NULL)
        {
                errnum = INDEXFILE_ERROR_INVALID_FILE;
                return errnum;
        }
        if(TestIndexWord(iword) == 0)
        {
                errnum = INDEXFILE_ERROR_INVALID_WORD;
                return errnum;
        }
#endif

        DWORD strlen_iword = (DWORD)strlen(iword); // just for optimizing

        if(strlen_iword > 1)
        {
                curpos = begin_pos;

                // prepare cache
                bcache = CacheROOT;

                if(strlen_iword > maxindex) indlength = maxindex;
                else indlength = strlen_iword;

                for(i = 0; i < indlength; i++)
                {
                        if(i < FIRST_LEVELS_CACHE_SIZE)
                        {
                                if(i != 0)
                                {
                                        if((DWORD)bcache->IM[cpos] == INDEXFILE_NO_INDEX)
                                        {
                                                // load to cache

                                                // allocate block
                                                bcache->IM[cpos] = AllocCacheBlock(symcount);

                                                // and read index from disk
                                                if(wcfseek(ifh, iblk[cpos].Index, SEEK_SET) != 0)
                                                {
                                                        unlock_and_indexer_io_error1();
                                                }
                                                if(wcfread(bcache->IM[cpos]->II, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
                                                {
                                                        unlock_and_indexer_io_error1();
                                                }

                                        }
                                        // get it from cache
                                        bcache = bcache->IM[cpos];
                                }
                                iblk = bcache->II;
                        }
                        else
                        {
                                iblk = iblock;
                                if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                                if(wcfread(iblk, 1, sizeof(SSymbolIndex)*symcount, ifh) != sizeof(SSymbolIndex)*symcount)
                                {
                                        unlock_and_indexer_io_error1();
                                }
                        }

                        cpos = symtable[(unsigned char)iword[i]];

                        if(iblk[cpos].Index == INDEXFILE_NO_INDEX)
                        {
                                errnum = INDEXFILE_ERROR_INDEX_NOT_EXIST;
                                return errnum;
                        }
                        else
                        {
                                if(i != indlength - 1)
                                {
                                        curpos = iblk[cpos].Index;
                                }
                        }
                }

                if(iblk[cpos].Location == INDEXFILE_NO_INDEX)
                {
                        errnum = INDEXFILE_ERROR_INDEX_NOT_EXIST;
                        return errnum;
                }
                else
                {
                        if(oldLocIndex != iblk[cpos].Location)
                        {
                                errnum = INDEXFILE_ERROR_INDEX_NOT_EXIST;
                                return errnum;
                        }
                        else
                        {
                                iblk[cpos].Location = NewLocIndex;
                                if(indlength > FIRST_LEVELS_CACHE_SIZE)
                                {
                                        // save index
                                        indexer_lock_file();
                                        if(wcfseek(ifh, curpos, SEEK_SET) != 0)
                                        {
                                                unlock_and_indexer_io_error1();
                                        }
                                        DWORD rr = (DWORD)wcfwrite(iblk, 1, symcount*sizeof(SSymbolIndex), ifh);
                                        indexer_unlock_file();

                                        if(rr != symcount*sizeof(SSymbolIndex))
                                        {
                                                indexer_io_error1();
                                        }
                                }

                                errnum = INDEXFILE_ERROR_ALLOK;
                                return errnum;
                        }
                }
        }
        else
        {
                errnum = INDEXFILE_ERROR_INVALID_WORD;
                return errnum;
        }
}
