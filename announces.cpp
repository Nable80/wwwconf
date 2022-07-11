/***************************************************************************
                          announces.cpp  -  board announces support
                             -------------------
    begin                : Mon Feb 24 2003
    copyright            : (C) 2001 - 2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "announces.h"
#include "error.h"

static FILE * CreateAnnounceFile()
{
        FILE *f;
        f = fopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_CW);
        if(f) {
                // announces number starting from 1
                DWORD x = 1;
                // write starting unique id
                if(fwrite(&x, 1, sizeof(x), f) != sizeof(x)) {
                        fclose(f);
                        f = NULL;
                }
                else {
                        fclose(f);
                        f = fopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_RW);
                }
        }
        return f;
}

int ReadLastAnnounceNumber(DWORD *an)
{
        FILE *f;
        int ret = 0;

        f = fopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_RW);
        if(!f) return 0;
        if(fread(an, 1, sizeof(DWORD), f) == sizeof(DWORD))
                ret = 1;
        fclose(f);
        return ret;
}

int PostGlobalAnnounce(char *username, DWORD uniqid, char *announce, DWORD ttl, DWORD flags)
{
        FILE *f;
        DWORD siz;
        SGlobalAnnounce an;
        DWORD id;

        if((f = wcfopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_RW)) == NULL) {
                if((f = CreateAnnounceFile()) == NULL) {
                        return ANNOUNCES_RETURN_IO_ERROR;
                }
        }
        // prepare announce structure
        strcpy(an.Announce, announce);
        strcpy(an.From, username);
        an.UIdFrom = uniqid;
        an.TTL = ttl;
        an.Date = time(NULL);
        an.Flags = flags;
        memset(an.Reserved, 0, sizeof(an.Reserved));
        // write announce
        lock_file(f);
        if(!fCheckedRead(&id, sizeof(id), f)) {
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_DB_ERROR;
        }
        // save number
        an.Number = id;
        // increment number
        id++;
        if(wcfseek(f, 0, SEEK_SET) != 0) {
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        if(!fCheckedWrite(&id, sizeof(id), f)) {
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        if(wcfseek(f, 0, SEEK_END) != 0) {
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        // start transcation
        siz = wcftell(f);
        if(!fCheckedWrite(&an, sizeof(SGlobalAnnounce), f)) {
                // rolling back
                if (truncate(F_GLOBAL_ANN, siz))
                        printhtmlerror();
                id--;
                if(wcfseek(f, 0, SEEK_END) == 0) {
                        fCheckedWrite(&id, sizeof(id), f);
                }
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        unlock_file(f);
        wcfclose(f);
        return ANNOUNCES_RETURN_OK;
}

int ReadGlobalAnnounces(SGlobalAnnounce **ga, DWORD *cnt)
{
        FILE *f;
        DWORD i = 0, rd;
        SGlobalAnnounce *an;

        *ga = NULL;
        *cnt = 0;

        an = (SGlobalAnnounce*)malloc(sizeof(SGlobalAnnounce));
        if(!an) return ANNOUNCES_RETURN_LOW_RES;

        if((f = fopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_R)) == NULL) {
                if((f = CreateAnnounceFile()) == NULL) {
                        free(an);
                        return ANNOUNCES_RETURN_IO_ERROR;
                }
        }
        if(wcfseek(f, sizeof(DWORD), SEEK_SET) != 0) {
                free(an);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        while(!feof(f))
        {
                if((rd = fread(&(an[i]), 1, sizeof(SGlobalAnnounce), f)) != sizeof(SGlobalAnnounce)) {
                        if(rd != 0) {
                                free(an);
                                fclose(f);
                                return ANNOUNCES_RETURN_DB_ERROR;
                        }
                        else break;
                }
                i++;
                an = (SGlobalAnnounce*)realloc(an, (i+1)*sizeof(SGlobalAnnounce));
        }
        fclose(f);
        *cnt = i;
        *ga = an;
        return ANNOUNCES_RETURN_OK;
}

int DeleteGlobalAnnounce(DWORD id, DWORD uniqid)
{
        WCFILE *f;
        DWORD pos, fsize, rd;
        SGlobalAnnounce *an;

        an = (SGlobalAnnounce*)malloc(sizeof(SGlobalAnnounce));
        if(!an) return ANNOUNCES_RETURN_LOW_RES;

        if((f = wcfopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_RW)) == NULL) {
                free(an);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        lock_file(f);
        if(wcfseek(f, sizeof(DWORD), SEEK_SET) != 0) {
                free(an);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        an->Number = 0xffffffff;
        pos = 0;
        while(!wcfeof(f))
        {
                pos = wcftell(f);
                if((rd = wcfread(an, 1, sizeof(SGlobalAnnounce), f)) != sizeof(SGlobalAnnounce)) {
                        if(rd != 0) {
                                free(an);
                                wcfclose(f);
                                return ANNOUNCES_RETURN_DB_ERROR;
                        }
                        else break;
                }
                if(an->Number == id && (uniqid == 0 || an->UIdFrom == uniqid)) break;
        }
        if(an->Number == id) {
                // let's delete it
                unsigned i = 0;
                while(!wcfeof(f))
                {
                        if((rd = wcfread(&(an[i]), 1, sizeof(SGlobalAnnounce), f)) != sizeof(SGlobalAnnounce)) {
                                if(rd != 0) {
                                        free(an);
                                        wcfclose(f);
                                        return ANNOUNCES_RETURN_DB_ERROR;
                                }
                                else break;
                        }
                        i++;
                        an = (SGlobalAnnounce*)realloc(an, (i+1)*sizeof(SGlobalAnnounce));
                }
                fsize = wcftell(f);
                fsize -= sizeof(SGlobalAnnounce);
                if(wcfseek(f, pos, SEEK_SET) != 0) {
                        free(an);
                        wcfclose(f);
                        return ANNOUNCES_RETURN_DB_ERROR;
                }
                if(wcfwrite(an, 1, sizeof(SGlobalAnnounce)*i, f) != sizeof(SGlobalAnnounce)*i) {
                        free(an);
                        wcfclose(f);
                        return ANNOUNCES_RETURN_DB_ERROR;
                }
                free(an);
                // !!!!!!!!!!!!
                wcfflush(f);
                // truncate end of file
                if (truncate(F_GLOBAL_ANN, fsize))
                        printhtmlerror();
                unlock_file(f);
                wcfclose(f);
        }
        else {
                // announce not found
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_NOT_FOUND;
        }

        return ANNOUNCES_RETURN_OK;
}

int UpdateGlobalAnnounce(DWORD id, char *username, DWORD uniqid, char *announce,
                DWORD ttl, DWORD flags, DWORD updateopt)
{
        WCFILE *f;
        DWORD pos, rd;
        SGlobalAnnounce *an;

        an = (SGlobalAnnounce*)malloc(sizeof(SGlobalAnnounce));
        if(!an) return ANNOUNCES_RETURN_LOW_RES;

        if((f = wcfopen(F_GLOBAL_ANN, FILE_ACCESS_MODES_RW)) == NULL) {
                free(an);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        lock_file(f);
        if(wcfseek(f, sizeof(DWORD), SEEK_SET) != 0) {
                free(an);
                return ANNOUNCES_RETURN_IO_ERROR;
        }
        an->Number = 0xffffffff;
        pos = 0;
        while(!wcfeof(f))
        {
                pos = wcftell(f);
                if((rd = wcfread(an, 1, sizeof(SGlobalAnnounce), f)) != sizeof(SGlobalAnnounce)) {
                        if(rd != 0) {
                                free(an);
                                wcfclose(f);
                                return ANNOUNCES_RETURN_DB_ERROR;
                        }
                        else break;
                }
                if(an->Number == id) break;
        }
        if(an->Number == id) {
                // let's update it
                strcpy(an->Announce, announce);
                if((updateopt & ANNOUNCES_UPDATE_OPT_USER)) {
                        strcpy(an->From, username);
                        an->UIdFrom = uniqid;
                }
                if((updateopt & ANNOUNCES_UPDATE_OPT_TTL)) an->TTL = ttl;
                if((updateopt & ANNOUNCES_UPDATE_OPT_TIME)) an->Date = time(NULL);
                if((updateopt & ANNOUNCES_UPDATE_OPT_FLAGS)) an->Flags = flags;
                memset(an->Reserved, 0, sizeof(an->Reserved));
                if(wcfseek(f, pos, SEEK_SET) != 0) {
                        free(an);
                        wcfclose(f);
                        return ANNOUNCES_RETURN_DB_ERROR;
                }
                if(wcfwrite(an, 1, sizeof(SGlobalAnnounce), f) != sizeof(SGlobalAnnounce)) {
                        free(an);
                        wcfclose(f);
                        return ANNOUNCES_RETURN_DB_ERROR;
                }
                free(an);
                unlock_file(f);
                wcfclose(f);
        }
        else {
                // announce not found
                unlock_file(f);
                wcfclose(f);
                return ANNOUNCES_RETURN_NOT_FOUND;
        }

        return ANNOUNCES_RETURN_OK;

}
