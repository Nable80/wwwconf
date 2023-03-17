/***************************************************************************
                          hashindex.cpp  -  hashed indexes for strings
                             -------------------
    begin                : Sun Nov 5 2002
    copyright            : (C) 2001-2002 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "hashindex.h"

/*  Index WCFILE format
 *  db 10, [name], 13, [Index=DWORD 4 bytes]
 *  ........................................
 */

static char* mstrstr(char *s, char *f, DWORD size)
{
        char *ss, *ff = f;
        DWORD i = 0;
        while(i <= size) {
                ss = s;
                ff = f;
                while(*s == *ff && *ff != 0) {
                        s++;
                        i++;
                        ff++;
                }
                if(*ff == 0) {
                        return ss;
                }
                if(s - ss == 0) {
                        s++;
                        i++;
                }
        }
        return NULL;
}

DWORD hashstr(const char *s, DWORD m)
{
        DWORD sum = 0;
        for(; *s != 0; s++) {
                sum += (DWORD)(*s & 0xFF);
        }
        return (sum % m);
}

int GetIndexOfString(char *s, DWORD *Index)
{
        WCFILE *f;
        DWORD hash;
        char buf[HASHINDEX_BLOCK_SIZE];
        char ps[MAX_HASHINDEX_STRLEN + 10];
        HASHINDEX_BLOCKINFO bi;
        char *fs;

        if(strlen(s) < 3 || strlen(s) > MAX_HASHINDEX_STRLEN)
                return HASHINDEX_ER_FORMAT;

        hash = HASHINDEX_BLOCK_SIZE * hashstr(s, HASHTAB_LEN);

        if((f = wcfopen(F_PROF_INDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return HASHINDEX_ER_IO_READ;

        for(;;) {
                if(wcfseek(f, hash, SEEK_SET) != 0) {
                        unlock_file(f);
                        return HASHINDEX_ER_IO_READ;
                }

                if(!fCheckedRead(buf, HASHINDEX_BLOCK_SIZE, f)) {
                        unlock_file(f);
                        return HASHINDEX_ER_IO_READ;
                }

                ps[0] = 10;
                strcpy(&ps[1], s);
                ps[strlen(s) + 1] = 13;
                ps[strlen(s) + 2] = 0;
                memcpy(&bi, buf, sizeof(bi));

                //
                //        Search for string in loaded block
                //
                if((fs = mstrstr(buf + sizeof(HASHINDEX_BLOCKINFO), ps,
                                 bi.Used - sizeof(HASHINDEX_BLOCKINFO))) != NULL) {
                        memcpy(Index, fs + strlen(s) + 2, 4);
                        break;
                }

                if(bi.Next == HASHINDEX_NULL) {
                        wcfclose(f);
                        return HASHINDEX_ER_NOT_FOUND;
                }
                else hash = bi.Next;
        }

        wcfclose(f);

        return HASHINDEX_ER_OK;
}

int AddStringToHashedIndex(const char *s, DWORD Index)
{
        WCFILE *f;
        DWORD oldhash, hash, neededsize, i;
        HASHINDEX_BLOCKINFO bi;
        char buf[HASHINDEX_BLOCK_SIZE];
        char prepbuf[MAX_HASHINDEX_STRLEN + 10];

        if(strlen(s) < 3 || strlen(s) > MAX_HASHINDEX_STRLEN)
                return HASHINDEX_ER_FORMAT;

        hash = HASHINDEX_BLOCK_SIZE * hashstr(s, HASHTAB_LEN);

        if((f = wcfopen(F_PROF_INDEX, FILE_ACCESS_MODES_RW)) == NULL) {
                //
                //        try to create new index WCFILE
                //
                if((f = wcfopen(F_PROF_INDEX, FILE_ACCESS_MODES_CW)) == NULL)
                        return HASHINDEX_ER_IO_CREATE;

                lock_file(f);

                memset(buf, 0, HASHINDEX_BLOCK_SIZE);
                bi.Used = sizeof(HASHINDEX_BLOCKINFO);
                bi.Next = HASHINDEX_NULL;
                memcpy(buf, &bi, sizeof(bi));

                for(i = 0; i < HASHTAB_LEN; i++) {
                        if(!fCheckedWrite(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                unlock_file(f);
                                wcfclose(f);
                                return HASHINDEX_ER_IO_WRITE;
                        }
                }

                unlock_file(f);
        }
        //
        //        Lock index WCFILE
        //
        lock_file(f);

        neededsize = strlen(s) + 4 /* for Index*/  + 2 /* for 10 and 13 signatures*/;
        prepbuf[0] = 10;
        strcpy(&prepbuf[1], s);
        prepbuf[neededsize - 5] = 13;
        prepbuf[neededsize - 4] = 0;
        prepbuf[neededsize] = 0;

        oldhash = 0xffffffff;
        for(;;) {
                if(wcfseek(f, hash, SEEK_SET) != 0) {
                        unlock_file(f);
                        wcfclose(f);
                        return HASHINDEX_ER_IO_READ;
                }

                if(!fCheckedRead(buf, HASHINDEX_BLOCK_SIZE, f)) {
                        unlock_file(f);
                        wcfclose(f);
                        return HASHINDEX_ER_IO_READ;
                }
                memcpy(&bi, buf, sizeof(bi));

                //
                //        Search for string in loaded block
                //
                if(mstrstr(buf+sizeof(bi), prepbuf,
                           bi.Used-sizeof(HASHINDEX_BLOCKINFO)) != NULL) {
                        unlock_file(f);
                        wcfclose(f);
                        return HASHINDEX_ER_ALREADY_EXIST;
                }

                if((oldhash == 0xffffffff) && (HASHINDEX_BLOCK_SIZE >= neededsize + bi.Used + 1)) {
                        // acceptable block - so save it for future use (write data, if not exist)
                        oldhash = hash;
                }

                if(bi.Next != HASHINDEX_NULL) {
                        //
                        //        Go ahead and check next chained block
                        //
                        hash = bi.Next;
                }
                else {
                        //
                        //        It was the last block - add new index now
                        //

                        // but in what block ;-)? so we should keep first acceptable block for write to

                        memcpy(&prepbuf[neededsize - 4], &Index, 4);

                        //
                        // check for space in current block
                        //
                        if(oldhash == 0xffffffff) {
                                DWORD newhash;
                                HASHINDEX_BLOCKINFO nbi;
                                //
                                //        Need to create new hash block and link it to the chain
                                //
                                if(wcfseek(f, 0, SEEK_END) != 0) {
                                        unlock_file(f);
                                        wcfclose(f);
                                        return HASHINDEX_ER_IO_READ;
                                }
                                newhash = wcftell(f);
                                memset(buf, 0, HASHINDEX_BLOCK_SIZE);
                                nbi.Next = HASHINDEX_NULL;
                                nbi.Used = (WORD)(neededsize + sizeof(HASHINDEX_BLOCKINFO));
                                memcpy(buf, &nbi, sizeof(nbi));
                                memcpy(&buf[sizeof(HASHINDEX_BLOCKINFO)], prepbuf, neededsize);
                                if(!fCheckedWrite(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                        unlock_file(f);
                                        wcfclose(f);
                                        return HASHINDEX_ER_IO_WRITE;
                                }
                                bi.Next = newhash;
                                if(wcfseek(f, hash, SEEK_SET) != 0) {
                                        unlock_file(f);
                                        wcfclose(f);
                                        return HASHINDEX_ER_IO_READ;
                                }
                                if(!fCheckedWrite(&bi, sizeof(bi), f)) {
                                        unlock_file(f);
                                        wcfclose(f);
                                        return HASHINDEX_ER_IO_WRITE;
                                }
                        }
                        else {
                                //
                                // Save to first suitable block
                                //
                                if(hash != oldhash) {
                                        hash = oldhash;
                                        if(wcfseek(f, hash, SEEK_SET) != 0) {
                                                unlock_file(f);
                                                wcfclose(f);
                                                return HASHINDEX_ER_IO_READ;
                                        }

                                        if(!fCheckedRead(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                                unlock_file(f);
                                                wcfclose(f);
                                                return HASHINDEX_ER_IO_READ;
                                        }
                                        memcpy(&bi, buf, sizeof(bi));
                                }

                                memcpy(&buf[bi.Used], prepbuf, neededsize + 1); // including term zero
                                bi.Used = (WORD)(bi.Used + neededsize);
                                if(wcfseek(f, hash, SEEK_SET) != 0) {
                                        unlock_file(f);
                                        wcfclose(f);
                                        return HASHINDEX_ER_IO_READ;
                                }
                                memcpy(buf, &bi, sizeof(bi));
                                if(!fCheckedWrite(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                        unlock_file(f);
                                        wcfclose(f);
                                        return HASHINDEX_ER_IO_WRITE;
                                }
                        }

                        break;
                }
        }

        unlock_file(f);
        wcfclose(f);

        return HASHINDEX_ER_OK;
}

int DeleteStringFromHashedIndex(const char *s)
{
        WCFILE *f;
        DWORD hash;
        HASHINDEX_BLOCKINFO bi;
        char buf[HASHINDEX_BLOCK_SIZE];
        char ps[MAX_HASHINDEX_STRLEN + 10];
        char *fs;

        if(strlen(s) < 3 || strlen(s) > MAX_HASHINDEX_STRLEN)
                return HASHINDEX_ER_FORMAT;

        hash = HASHINDEX_BLOCK_SIZE * hashstr(s, HASHTAB_LEN);

        if((f = wcfopen(F_PROF_INDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return HASHINDEX_ER_IO_READ;
        lock_file(f);

        for(;;) {
                if(wcfseek(f, hash, SEEK_SET) != 0) {
                        unlock_file(f);
                        return HASHINDEX_ER_IO_READ;
                }

                if(!fCheckedRead(buf, HASHINDEX_BLOCK_SIZE, f)) {
                        unlock_file(f);
                        return HASHINDEX_ER_IO_READ;
                }

                ps[0] = 10;
                strcpy(&ps[1], s);
                ps[strlen(s) + 1] = 13;
                ps[strlen(s) + 2] = 0;
                memcpy(&bi, buf, sizeof(bi));

                //
                //        Search for string in loaded block
                //
                if((fs = mstrstr(buf + sizeof(HASHINDEX_BLOCKINFO), ps,
                                 bi.Used - sizeof(HASHINDEX_BLOCKINFO))) != NULL) {
                        DWORD len = strlen(s) + 2 + 4;
                        //
                        //        remove from block
                        //
                        for(; (fs - buf) < bi.Used; *fs = *(fs + len), fs++);
                        memset(&(buf[bi.Used - len-1]), 0, len+1);

                        if(wcfseek(f, hash, SEEK_SET) != 0) {
                                unlock_file(f);
                                return HASHINDEX_ER_IO_READ;
                        }
                        bi.Used = (WORD)(bi.Used - len);
                        memcpy(buf, &bi, sizeof(bi));

                        if(!fCheckedWrite(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                unlock_file(f);
                                return HASHINDEX_ER_IO_READ;
                        }

                        break;
                }

                if(bi.Next == HASHINDEX_NULL) {
                        unlock_file(f);
                        wcfclose(f);
                        return HASHINDEX_ER_NOT_FOUND;
                }
                else hash = bi.Next;
        }

        unlock_file(f);
        wcfclose(f);

        return HASHINDEX_ER_OK;
}

int GenerateHashwordList(char **names)
{
        WCFILE *f;
        DWORD hash, pos, curalloced, bb = 0;
        char buf[HASHINDEX_BLOCK_SIZE];
        HASHINDEX_BLOCKINFO bi;
        char *fs, *ss;

#define GHL_REALLOC_BLOCK_SIZE 20000
        *names = ss = (char*)malloc(GHL_REALLOC_BLOCK_SIZE);
        if (!ss) {
                return HASHINDEX_ER_IO_READ;
        }
        *ss = 0;
        curalloced = GHL_REALLOC_BLOCK_SIZE;

        if((f = wcfopen(F_PROF_INDEX, FILE_ACCESS_MODES_R)) == NULL)
                return HASHINDEX_ER_IO_CREATE;

        for(hash = 0; hash < HASHTAB_LEN; hash++) {
                pos = hash*HASHINDEX_BLOCK_SIZE;
                for(;;) {
                        if(wcfseek(f, pos, SEEK_SET) != 0) {
                                unlock_file(f);
                                return HASHINDEX_ER_IO_READ;
                        }

                        if(!fCheckedRead(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                free(*names);
                                unlock_file(f);
                                return HASHINDEX_ER_IO_READ;
                        }

                        memcpy(&bi, buf, sizeof(bi));

                        //
                        // parse block
                        //
                        fs = buf + sizeof(bi);
                        while(*fs != 0) {
                                char *d;
                                fs++;
                                d = fs;
                                while(*d != 13) d++;
                                *d = 0;

                                // save result
                                strcat(ss, fs);
                                ss += strlen(fs) + 1;

                                // increment used memory
                                bb += strlen(fs) + 1;

                                *(ss - 1) = 13;
                                *ss = 0;
                                if(bb >= curalloced - 40) {
                                        curalloced += GHL_REALLOC_BLOCK_SIZE;
                                        *names = (char*)realloc(*names, curalloced);
                                }
                                d += 5;
                                fs = d;
                        }

                        if(bi.Next != HASHINDEX_NULL)
                                pos = bi.Next;
                        else break;
                }
        }

        wcfclose(f);

        return HASHINDEX_ER_OK;
}

static int compare(const void *p1, const void *p2)
{
        return int(*((DWORD*)p1) - *((DWORD*)p2));
}

int GenerateIndexList(DWORD **index)
{
        WCFILE *f;
        DWORD hash, pos, curalloced, si = 0        /* saving index */;
        char buf[HASHINDEX_BLOCK_SIZE];
        HASHINDEX_BLOCKINFO bi;
        char *fs;

        if((f = wcfopen(F_PROF_INDEX, FILE_ACCESS_MODES_R)) == NULL)
                return HASHINDEX_ER_IO_CREATE;

#define GIL_REALLOC_BLOCK_SIZE 2000*sizeof(DWORD)
        *index = (DWORD*)malloc(GIL_REALLOC_BLOCK_SIZE);
        if (*index == NULL) {
                wcfclose(f);
                return HASHINDEX_ER_IO_READ;
        }
        curalloced = GIL_REALLOC_BLOCK_SIZE;

        for(hash = 0; hash < HASHTAB_LEN; hash++) {
                pos = hash*HASHINDEX_BLOCK_SIZE;
                for(;;) {
                        if(wcfseek(f, pos, SEEK_SET) != 0) {
                                free(*index);
                                *index = NULL;
                                unlock_file(f);
                                wcfclose(f);
                                return HASHINDEX_ER_IO_READ;
                        }

                        if(!fCheckedRead(buf, HASHINDEX_BLOCK_SIZE, f)) {
                                free(*index);
                                *index = NULL;
                                unlock_file(f);
                                wcfclose(f);
                                return HASHINDEX_ER_IO_READ;
                        }

                        memcpy(&bi, buf, sizeof(bi));

                        //
                        // parse block
                        //
                        fs = buf + sizeof(bi);
                        while(*fs != 0) {
                                char *d;
                                fs++;
                                d = fs;
                                while(*d != 13 && *d != 0) d++;

                                if(*d == 0) break;

                                d++;

                                DWORD val;
                                memcpy(&val, d, sizeof(val));
                                (*index)[si++] = val;

                                if(si*sizeof(DWORD) >= curalloced - 40) {
                                        curalloced += GHL_REALLOC_BLOCK_SIZE;
                                        *index = (DWORD*)realloc(*index, curalloced);
                                }
                                d += 4;
                                fs = d;
                        }

                        if(bi.Next != HASHINDEX_NULL)
                                pos = bi.Next;
                        else break;
                }
        }

        wcfclose(f);

        qsort(((void*)(*index)), si, sizeof(DWORD), compare);
        (*index)[si] = 0xffffffff;        // end of array

        return HASHINDEX_ER_OK;
}
