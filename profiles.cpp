/***************************************************************************
                          profiles.h  -  profiles support
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "hashindex.h"
#include "profiles.h"
#include "freedb.h"
#include "error.h"

/* return 1 if valid, 0 otherwise
 */
int isLoginStrValid(char *s)
{
        while(*s != 0) {
                if(((unsigned char)(*s)) < 32) {
                        return 0;
                }
                s++;
        }
        if(strlen(s) > PROFILES_MAX_USERNAME_LENGTH - 1) return 0;
        return 1;
}

/* constructor */
CProfiles::CProfiles()
{
        errnum = PROFILE_RETURN_ALLOK;
}


/* constructor */
CProfiles::~CProfiles()
{
}

/* write SProfile_FullUserInfo structure, allocating space for it automatically
 * return 1 if successfull, otherwise zero 
 */
int CProfiles::WriteFullInfo(DWORD *idx, SProfile_FullUserInfo *FI)
{
        /* prepare about string length */
        if(FI->AboutUser != NULL)
                FI->size = (DWORD)strlen(FI->AboutUser);
        else
                FI->size = 0;

        /* alloc free space */
        CFreeDBFile fdf(F_PROF_FREEBODY, PROFILE_WASTED_FINFO_SIZE);
        if(fdf.errnum != FREEDBFILE_ERROR_ALLOK)
                return 0;

        if((*idx = fdf.AllocFreeSpace(sizeof(SProfile_FullUserInfo) - sizeof(char*) +
                FI->size)) == 0xFFFFFFFF) {
                if(fdf.errnum != FREEDBFILE_ERROR_ALLOK) 
                        return 0;
                
                if(wcfseek(Fp_b, 0, SEEK_END) != 0)
                        return 0;
                *idx = wcftell(Fp_b);
        }
        else {
                if(wcfseek(Fp_b, *idx, SEEK_SET) != 0)
                        return 0;
        }

        /* write SProfile_FullUserInfo */
        if(!fCheckedWrite(FI, sizeof(SProfile_FullUserInfo) - sizeof(char*), Fp_b))
                return 0;
        if(!fCheckedWrite(FI->AboutUser, FI->size, Fp_b))
                return 0;

        return 1;
}
/* read SProfile_FullUserInfo structure 
 * return 1 if successfull, otherwise zero 
 */
int CProfiles::ReadFullInfo(DWORD idx, SProfile_FullUserInfo *FI)
{
        // read SProfile_FullUserInfo
        if(wcfseek(Fp_b, idx, SEEK_SET) != 0)
                return 0;
        if(!fCheckedRead(FI, sizeof(SProfile_FullUserInfo) - sizeof(char*), Fp_b))
                return 0;
        
        // prepare "about" string length
        FI->AboutUser = (char*)malloc(FI->size + 1);

        if(!fCheckedRead(FI->AboutUser, FI->size, Fp_b)) {
                free(FI->AboutUser);
                return 0;
        }
        // set final zero at the end of string
        FI->AboutUser[FI->size] = 0;

        return 1;
}
/* delete SProfile_FullUserInfo structure from profile body database
 * by index [idx] and mark space as free
 * return 1 if successfull, otherwise zero 
 */
int CProfiles::DeleteFullInfo(DWORD idx)
{
        /* read old Full Info */
        SProfile_FullUserInfo *ofi = (SProfile_FullUserInfo*)malloc(sizeof(SProfile_FullUserInfo));

        if(ReadFullInfo(idx, ofi) == 0) {
                free(ofi);
                return 0;
        }

        /* mark free space */
        CFreeDBFile fdf(F_PROF_FREEBODY, PROFILE_WASTED_FINFO_SIZE);
        if(fdf.errnum != FREEDBFILE_ERROR_ALLOK) {
                free(ofi->AboutUser);
                free(ofi);
                return 0;
        }
        DWORD rr = sizeof(SProfile_FullUserInfo) - sizeof(char*) + ofi->size;
        free(ofi->AboutUser);
        free(ofi);
        if(fdf.MarkFreeSpace(idx, rr) != FREEDBFILE_ERROR_ALLOK)
                return 0;

        return 1;
}


/* read count structures SProfile_UserInfo at index idx */
int CProfiles::ReadUInfo(DWORD idx, SProfile_UserInfo *FI)
{
        if(wcfseek(Fp_i, idx, SEEK_SET) != 0)
                return 0;
        if(!fCheckedRead(FI, sizeof(SProfile_UserInfo), Fp_i))
                return 0;

        return 1;
}

/* write new info structure SProfile_UserInfo and return it index in *idx
 * and seek to the find place 
 * DESTROY curent position of Fp_i file !!!
 * return 1 if successfull, otherwise zero returned
 */
int CProfiles::GetSpaceforUInfo(DWORD *idx)
{
        /* alloc free space */
        CFreeDBFile fdf(F_PROF_FREENIDX, 0);
        if(fdf.errnum != FREEDBFILE_ERROR_ALLOK)
                return 0;

        if((*idx = fdf.AllocFreeSpace(sizeof(SProfile_UserInfo))) == 0xFFFFFFFF) {
                if(fdf.errnum != FREEDBFILE_ERROR_ALLOK)
                        return 0;
                
                if(wcfseek(Fp_i, 0, SEEK_END) != 0)
                        return 0;
                *idx = wcftell(Fp_i);
        }
        else {
                if(wcfseek(Fp_i, *idx, SEEK_SET) != 0)
                        return 0;
        }

        return 1;
}

/* write count structures SProfile_UserInfo at index idx */
int CProfiles::WriteUInfo(DWORD idx, SProfile_UserInfo *FI)
{
        if(wcfseek(Fp_i, idx, SEEK_SET) != 0)
                return 0;
        if(!fCheckedWrite(FI, sizeof(SProfile_UserInfo), Fp_i))
                return 0;

        return 1;
}

int CProfiles::DeleteUInfo(DWORD idx)
{
        /* mark free space */
        CFreeDBFile fdf(F_PROF_FREENIDX, sizeof(SProfile_UserInfo));
        if(fdf.errnum != FREEDBFILE_ERROR_ALLOK)        
                return 0;
        if(fdf.MarkFreeSpace(idx, sizeof(SProfile_UserInfo)) != FREEDBFILE_ERROR_ALLOK)
                return 0;

        return 1;
}


/* add new user to profile database
 * return PROFILE_RETURN_ALLOK if successfull, otherwise standart error codes 
 */
int CProfiles::AddNewUser(SProfile_UserInfo *newprf, SProfile_FullUserInfo *FullUI, DWORD *ui_index)
{
        int ret, i;
        DWORD idx;
        DWORD rr;
        DWORD ucount;
        Fp_i = NULL;
        Fp_b = NULL;

        if(!isLoginStrValid(newprf->username) || strlen(newprf->username) < 3) {
                return PROFILE_RETURN_INVALID_LOGIN;
        }

        if((ret = GetIndexOfString(newprf->username, &rr)) == HASHINDEX_ER_NOT_FOUND ||
                ret == HASHINDEX_ER_IO_READ) {
                /* prepare SProfile_UserInfo and sb structures */
                newprf->postcount = 0;
                FullUI->CreateDate = time(NULL);
                newprf->LoginDate = 0;
                newprf->persmsg = 0xffffffff;
                newprf->persmescnt = 0;
                newprf->readpersmescnt = 0;
                newprf->postedmescnt = 0;
                newprf->postedpersmsg = 0xffffffff;
                newprf->RefreshCount = 0;


                newprf->vs.dsm = CONFIGURE_SETTING_DEFAULT_dsm;
                newprf->vs.topics = CONFIGURE_SETTING_DEFAULT_topics;
                newprf->vs.tv = CONFIGURE_SETTING_DEFAULT_tv;
                newprf->vs.tc = CONFIGURE_SETTING_DEFAULT_tc;
                newprf->vs.ss = CONFIGURE_SETTING_DEFAULT_ss;
                newprf->vs.lsel = CONFIGURE_SETTING_DEFAULT_lsel;
                newprf->vs.tt = CONFIGURE_SETTING_DEFAULT_tt;
                newprf->vs.tz = DATETIME_DEFAULT_TIMEZONE;
                
                // New status = 0
                newprf->Status = 0;
                for( i=0; i<PROFILES_FAV_THREADS_COUNT; i++)
                        newprf->favs[i]=0;
                        
                /* create new index */
                if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                        goto Unlock_and_return;
                if((Fp_b = wcfopen(F_PROF_BODY, FILE_ACCESS_MODES_RW)) == NULL)
                        goto Unlock_and_return;
                
                /********* lock Fp_i, Fp_b files *********/
                lock_file(Fp_i);
                lock_file(Fp_b);
                
                // read unique user ID
                if(!fCheckedRead(&(newprf->UniqID), sizeof(newprf->UniqID), Fp_i))
                        goto Unlock_and_return;
                (newprf->UniqID)++;
                
                // read user count
                if(!fCheckedRead(&ucount, sizeof(ucount), Fp_i))
                        goto Unlock_and_return;
                // increment user count
                ucount++;
                
                // get free index for user info structure
                if(GetSpaceforUInfo(&idx) == 0)
                        goto Unlock_and_return;
                
                if(ui_index != NULL) *ui_index = idx;

                if(wcfseek(Fp_i, idx, SEEK_SET) != 0)
                        goto Unlock_and_return;

                // write SProfile_FullUserInfo
                if(WriteFullInfo(&(newprf->FullInfo_ID), FullUI) == 0) {
                        goto Unlock_and_return;
                }

                // write SProfile_UserInfo
                if(WriteUInfo(idx, newprf) == 0) {
                        goto Unlock_and_return;
                }

                if(AddStringToHashedIndex(newprf->username, idx) != HASHINDEX_ER_OK) {
                        DeleteUInfo(idx);
                        DeleteFullInfo(newprf->FullInfo_ID);
                        goto Unlock_and_return;
                }

                // write new unique user ID and current user count
                if(wcfseek(Fp_i, 0, SEEK_SET) != 0)
                        goto Unlock_and_return;
                if(!fCheckedWrite(&(newprf->UniqID), sizeof(newprf->UniqID), Fp_i))
                        goto Unlock_and_return;
                if(!fCheckedWrite(&ucount, sizeof(ucount), Fp_i))
                        goto Unlock_and_return;
                
                unlock_file(Fp_i);
                unlock_file(Fp_b);
                /********* unlock Fp_i, Fp_b files *********/
                
                wcfclose(Fp_i);
                wcfclose(Fp_b);
                return PROFILE_RETURN_ALLOK;
                
Unlock_and_return:
                // error - unlock and exit
                if(Fp_i) {
                        unlock_file(Fp_i);
                        wcfclose(Fp_i);
                }
                if(Fp_b) {
                        unlock_file(Fp_b);
                        wcfclose(Fp_b);
                }
                return PROFILE_RETURN_DB_ERROR;
        }

        return PROFILE_RETURN_ALREADY_EXIST;
}

/* delete user [name] from profile database
 * return PROFILE_RETURN_ALLOK if successfull, otherwise standart error codes 
 */
int CProfiles::DeleteUser(char *name)
{
        DWORD idx, ucount;
        SProfile_UserInfo *pi;
        Fp_i = NULL;
        Fp_b = NULL;

        int ret = GetIndexOfString(name, &idx);

        switch(ret) {
        case HASHINDEX_ER_NOT_FOUND:
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_OK:
                pi = (SProfile_UserInfo*)malloc(sizeof(SProfile_UserInfo));

                // index exist - try to read block
                if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                        goto Do_Exit;

                if((Fp_b = wcfopen(F_PROF_BODY, FILE_ACCESS_MODES_RW)) == NULL)
                        goto Do_Exit;

                /********* lock Fp_i, Fp_b *********/
                lock_file(Fp_i);
                lock_file(Fp_b);

                if(wcfseek(Fp_i, idx, SEEK_SET) != 0)
                        goto Do_Exit;

                if(!fCheckedRead(pi, sizeof(SProfile_UserInfo), Fp_i))
                        goto Do_Exit;

                if(strcmp(pi->username, name) == 0) {
                        if(DeleteFullInfo(pi->FullInfo_ID) == 0)
                                goto Do_Exit;

                        if(DeleteUInfo(idx) == 0)
                                goto Do_Exit;

                        if(DeleteStringFromHashedIndex(name) != HASHINDEX_ER_OK)
                                goto Do_Exit;

                        // update user count
                        if(wcfseek(Fp_i, sizeof(DWORD), SEEK_SET) != 0)
                                goto Do_Exit;
                        if(!fCheckedRead(&ucount, sizeof(ucount), Fp_i))
                                goto Do_Exit;
                        ucount++;
                        if(wcfseek(Fp_i, sizeof(DWORD), SEEK_SET) != 0)
                                goto Do_Exit;
                        if(!fCheckedWrite(&ucount, sizeof(ucount), Fp_i))
                                goto Do_Exit;

                        unlock_file(Fp_i);
                        unlock_file(Fp_b);
                        /******** unlock Fp_i, Fp_b ********/

                        wcfclose(Fp_i);
                        wcfclose(Fp_b);

                        free(pi);
                        return PROFILE_RETURN_ALLOK;
                }

                unlock_file(Fp_i);
                unlock_file(Fp_b);
                /******** unlock Fp_i, Fp_b ********/

                wcfclose(Fp_i);
                wcfclose(Fp_b);

                free(pi);
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_FORMAT:
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_IO_READ:
        case HASHINDEX_ER_IO_CREATE:
                goto Do_Exit;

        default:
                return PROFILE_RETURN_UNKNOWN_ERROR;
        }

Do_Exit:

        if(Fp_i) {
                unlock_file(Fp_i);
                wcfclose(Fp_i);
        }
        if(Fp_b) {
                unlock_file(Fp_b);
                wcfclose(Fp_b);
        }

        return PROFILE_RETURN_DB_ERROR;
}

/* modify structures, user  if user exist, UserName cannot be changed
 * return PROFILE_RETURN_ALLOK if successfull, otherwise standart error codes
 * also if Fui is NULL, ModifyUser() will not modify Full User Information
 */
int CProfiles::ModifyUser(SProfile_UserInfo *newprf, SProfile_FullUserInfo *FullUI, DWORD *ui_index)
{
        DWORD idx;
        SProfile_UserInfo *pi;
        Fp_i = NULL;
        Fp_b = NULL;

        if(!isLoginStrValid(newprf->username) || strlen(newprf->username) < 3)
                return PROFILE_RETURN_INVALID_LOGIN;

        int ret = GetIndexOfString(newprf->username, &idx);

        switch(ret) {
        case HASHINDEX_ER_NOT_FOUND:
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_OK:
                pi = (SProfile_UserInfo*)malloc(sizeof(SProfile_UserInfo));

                // index exist - try to read block
                if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                        goto Do_Exit;

                if((Fp_b = wcfopen(F_PROF_BODY, FILE_ACCESS_MODES_RW)) == NULL)
                        goto Do_Exit;
                
                /********* lock Fp_i, Fp_b *********/
                lock_file(Fp_i);
                lock_file(Fp_b);

                if(wcfseek(Fp_i, idx, SEEK_SET) != 0)
                        goto Do_Exit;
                        
                if(!fCheckedRead(pi, sizeof(SProfile_UserInfo), Fp_i))
                        goto Do_Exit;

                if(strcmp(pi->username, newprf->username) == 0) {

                        if(wcfseek(Fp_i, idx, SEEK_SET) != 0) goto Do_Exit;
                        
                        if(ui_index != NULL) *ui_index = idx;

                        if(FullUI != NULL) {
                                // delete old and save new full info
                                if(DeleteFullInfo(pi->FullInfo_ID) == 0)
                                        goto Do_Exit;

                                if(WriteFullInfo(&(newprf->FullInfo_ID), FullUI) == 0)
                                        goto Do_Exit;
                        }
                        else {
                                newprf->FullInfo_ID = pi->FullInfo_ID;
                        }

                        // save old dinamic board information
                        newprf->postcount = pi->postcount;
                        newprf->UniqID = pi->UniqID;
                        newprf->persmsg = pi->persmsg;
                        newprf->lastIP = pi->lastIP;
                        newprf->persmescnt = pi->persmescnt;
                        newprf->readpersmescnt = pi->readpersmescnt;
                        newprf->postedmescnt = pi->postedmescnt;
                        newprf->postedpersmsg = pi->postedpersmsg;
                        newprf->RefreshCount = pi->RefreshCount;

                        // and finally save user profile
                        if(!fCheckedWrite(newprf, sizeof(SProfile_UserInfo), Fp_i))
                                goto Do_Exit;

                        unlock_file(Fp_i);
                        unlock_file(Fp_b);
                        /******** unlock Fp_i, Fp_b ********/

                        wcfclose(Fp_i);
                        wcfclose(Fp_b);

                        free(pi);
                        return PROFILE_RETURN_ALLOK;
                }

                unlock_file(Fp_i);
                unlock_file(Fp_b);
                /******** unlock Fp_i, Fp_b ********/

                wcfclose(Fp_i);
                wcfclose(Fp_b);

                free(pi);
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_FORMAT:
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_IO_READ:
        case HASHINDEX_ER_IO_CREATE:
                goto Do_Exit;

        default:
                return PROFILE_RETURN_UNKNOWN_ERROR;
        }

Do_Exit:

        if(Fp_i) {
                unlock_file(Fp_i);
                wcfclose(Fp_i);
        }
        if(Fp_b) {
                unlock_file(Fp_b);
                wcfclose(Fp_b);
        }

        return PROFILE_RETURN_DB_ERROR;
}

/* check, if there is user with same name in profile database
 * return PROFILE_RETURN_ALLOK if successfull, otherwise standart error codes
 * also if ui not NULL return SProfile_UserInfo for found user, and if also Fui not NULL
 * return Fui information too
 */
int CProfiles::GetUserByName(char *name, SProfile_UserInfo *ui, SProfile_FullUserInfo *Fui, DWORD *ui_index)
{
        DWORD idx;
        SProfile_UserInfo *pi;

        int ret = GetIndexOfString(name, &idx);
        // error code returned - next strings - analizing it

        switch(ret) {
        case HASHINDEX_ER_NOT_FOUND:
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_OK:
                pi = (SProfile_UserInfo*)malloc(sizeof(SProfile_UserInfo));

                // index exist - try to read block
                if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_R)) == NULL)
                        goto Do_Exit;

                if((Fp_b = wcfopen(F_PROF_BODY, FILE_ACCESS_MODES_R)) == NULL)
                        goto Do_Exit;
                
                if(wcfseek(Fp_i, idx, SEEK_SET) != 0)
                        goto Do_Exit;

                if(!fCheckedRead(pi, sizeof(SProfile_UserInfo), Fp_i))
                        goto Do_Exit;

                if(strcmp(pi->username, name) == 0) {
                        // profile found
                                        
                        if(ui != NULL) {
                                memcpy(ui, pi, sizeof(*pi));
                        }
                        if(Fui != NULL) {
                                // read SProfile_FullUserInfo
                                if(ReadFullInfo(pi->FullInfo_ID, Fui) == 0)
                                        goto Do_Exit;
                        }
                        if(ui_index != NULL) *ui_index = idx;

                        free(pi);
                        wcfclose(Fp_b);
                        wcfclose(Fp_i);
                        return PROFILE_RETURN_ALLOK;
                        
                }

                /* name not found - not exist */
                free(pi);
                wcfclose(Fp_b);
                wcfclose(Fp_i);
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_FORMAT:
                return PROFILE_RETURN_INVALID_LOGIN;

        case HASHINDEX_ER_IO_READ:
        case HASHINDEX_ER_IO_CREATE:
                goto Do_Exit;

        default:
                return PROFILE_RETURN_UNKNOWN_ERROR;
        }

Do_Exit:

        return PROFILE_RETURN_DB_ERROR;
}

int CProfiles::GetUsersCount(DWORD *uc)
{
        if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;

        if(wcfseek(Fp_i, sizeof(DWORD), SEEK_SET) != 0)
                return 0;
        if(!fCheckedRead(uc, sizeof(DWORD), Fp_i))
                return 0;

        wcfclose(Fp_i);
        return 1;
}

int CProfiles::GetUInfo(DWORD idx, SProfile_UserInfo *FI)
{
        if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_R)) == NULL)
                return 0;
        DWORD x = (ReadUInfo(idx, FI) != 1);
        wcfclose(Fp_i);
        if(x) return 0;
        return 1;
}

int CProfiles::SetUInfo(DWORD idx, SProfile_UserInfo *FI)
{
        if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;
        lock_file(Fp_i);
        DWORD x = (WriteUInfo(idx, FI) != 1);
        unlock_file(Fp_i);
        wcfclose(Fp_i);
        if(x) return 0;
        return 1;
}

int CProfiles::GetFullInfo(DWORD idx, SProfile_FullUserInfo *FI)
{
        if((Fp_b = wcfopen(F_PROF_BODY, FILE_ACCESS_MODES_R)) == NULL)
                return 0;
        DWORD x = (ReadFullInfo(idx, FI) != 1);
        wcfclose(Fp_b);
        if(x) return 0;
        return 1;
}

int CProfiles::GenerateUserList(char ***buf, DWORD *cnt)
{
        DWORD *ii;
        DWORD readed, i, curii = 0, c = 0, ac /* alloced count */, allc = 0;
#define ULIST_PROFILE_READ_COUNT 400
        SProfile_UserInfo pi[ULIST_PROFILE_READ_COUNT];

        *buf = NULL;
        *cnt = 0;
        if(GenerateIndexList(&ii) != HASHINDEX_ER_OK)
                return 0;

        if(ii[0] != 0xffffffff) {
                *buf = (char**)malloc(ULIST_PROFILE_READ_COUNT*sizeof(char**));
                if(!(*buf)) return 0;
                ac = ULIST_PROFILE_READ_COUNT;

                if((Fp_i = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_R)) == NULL) {
                        free(*buf);
                        *buf = NULL;
                        return 0;
                }

                if(wcfseek(Fp_i, ii[0], SEEK_SET) != 0)
                        return 0;

                do {
                        readed = (DWORD)wcfread(&pi, 1, sizeof(SProfile_UserInfo)*ULIST_PROFILE_READ_COUNT, Fp_i);
                        if((readed % sizeof(SProfile_UserInfo)) != 0) {
                                wcfclose(Fp_i);
                                for(i = 0; i < c; i++) {
                                        free((*buf)[i]);
                                }
                                free(*buf);
                                *buf = NULL;
                                return 0;
                        }
                        readed = readed / sizeof(SProfile_UserInfo);
                        
                        for(i = 0; i < readed; i++) {
                                // find this in our indexes (test for deleted profile)
                                while(ii[curii] < ((i + allc*ULIST_PROFILE_READ_COUNT)*sizeof(SProfile_UserInfo) + ii[0])) curii++;
                                if(ii[curii] == ((i + allc*ULIST_PROFILE_READ_COUNT)*sizeof(SProfile_UserInfo) + ii[0])) {
                                        // modify right - reset all right if SUPERUSER
                                        if((pi[i].right & USERRIGHT_SUPERUSER))
                                                pi[i].right = USERRIGHT_SUPERUSER;
                                        // store it
                                        size_t len = strlen(pi[i].username) + 1;
                                        (*buf)[c] = (char*)malloc(len + 5*sizeof(DWORD) + 1);
                                        memcpy(((char*)((*buf)[c])), &pi[i].lastIP, sizeof(DWORD));
                                        memcpy(((char*)((*buf)[c])) + 4, &pi[i].postcount, sizeof(DWORD));
                                        memcpy(((char*)((*buf)[c])) + 8, &pi[i].LoginDate, sizeof(DWORD));
                                        memcpy(((char*)((*buf)[c])) + 12, &pi[i].RefreshCount, sizeof(DWORD));
                                        memcpy(((char*)((*buf)[c])) + 16, &pi[i].right, sizeof(DWORD));
                                        memcpy(((char*)((*buf)[c])) + 20, &pi[i].username, len);
                                        c++;
                                        // check for realloc
                                        if(ac == c) {
                                                ac += ULIST_PROFILE_READ_COUNT;
                                                *buf = (char**)realloc(*buf, ac*sizeof(char**));
                                        }
                                }
                                // FOR DEBUG !!!
                                /*else {
                                        if(curii > 3) {
                                                for(int j = -3; j < 4; j++) {
                                                        print2log("%d", ii[curii + j]);
                                                }
                                                print2log("i=%d, name=%s, Done\n", (i + allc*ULIST_PROFILE_READ_COUNT)*sizeof(SProfile_UserInfo) + ii[0], pi[i].username);
                                        }
                                }*/
                        }
                        allc++;
                } while(readed == ULIST_PROFILE_READ_COUNT);

                wcfclose(Fp_i);

                free(ii);
                // realloc to the real size
                *buf = (char**)realloc(*buf, c*sizeof(char**));

                *cnt = c;
        }

        return 1;
}

int CProfiles::PostPersonalMessage(char *username, DWORD userindex, char *message, char *from, DWORD userindexfrom)
{
        SProfile_UserInfo ui, poster_ui;
        int ret;
        WCFILE *fp;

        // load recipient user profile
        if(username != NULL && strcmp(username, "") != 0) {
                // use username
                if((ret = GetUserByName(username, &ui, NULL, &userindex)) != PROFILE_RETURN_ALLOK)
                        return ret;
        }
        else {
                // use userindex
                if(!GetUInfo(userindex, &ui))
                        return PROFILE_RETURN_DB_ERROR;
        }

        // load sender user profile
        if(from != NULL && strcmp(from, "") != 0) {
                // use username
                if((ret = GetUserByName(from, &poster_ui, NULL, &userindexfrom)) != PROFILE_RETURN_ALLOK)
                        return ret;
        }
        else {
                // use userindex
                if(!GetUInfo(userindexfrom, &poster_ui))
                        return PROFILE_RETURN_DB_ERROR;
        }

        // prepare personal message structure
        SPersonalMessage mes;
        DWORD pos;
        mes.Date = time(NULL);
        strcpy(mes.NameFrom, poster_ui.username);
        mes.UIdFrom = poster_ui.UniqID;
        strcpy(mes.NameTo, ui.username);
        mes.UIdTo = ui.UniqID;
        strcpy(mes.Msg, message);
        mes.Prev = ui.persmsg;
        mes.PosterPrev = poster_ui.postedpersmsg;

        if((fp = wcfopen(F_PROF_PERSMSG, FILE_ACCESS_MODES_RW)) == NULL)
                return PROFILE_RETURN_DB_ERROR;
        lock_file(fp);
        // write to the file
        if(wcfseek(fp, 0, SEEK_END) != 0)
                goto PostPersMsg_Error;
        pos = wcftell(fp);
        if(!fCheckedWrite(&mes, sizeof(mes), fp))
                goto PostPersMsg_Error;

        // update recipient user profile
        ui.persmsg = pos;
        ui.persmescnt++;
        SetUInfo(userindex, &ui);

        // to correct bug with post to youself
        if(poster_ui.UniqID == ui.UniqID)
                memcpy(&poster_ui, &ui, sizeof(SProfile_UserInfo));

        // update sender user profile
        poster_ui.postedpersmsg = pos;
        poster_ui.postedmescnt++;
        SetUInfo(userindexfrom, &poster_ui);

        unlock_file(fp);
        wcfclose(fp);
        return PROFILE_RETURN_ALLOK;

PostPersMsg_Error:
        unlock_file(fp);
        wcfclose(fp);
        return PROFILE_RETURN_DB_ERROR;
}

int CProfiles::ReadPersonalMessages(char *username, DWORD userindex,
                                                                        SPersonalMessage **tomessages, DWORD *tocount,
                                                                        SPersonalMessage **frommessages, DWORD *fromcount)
{
        SProfile_UserInfo ui;
        int ret;
        WCFILE *fp;
        SPersonalMessage *msg;
        DWORD toread, i, fromread, curpos;

        if(username != NULL && strcmp(username, "") != 0) {
                // use username
                if((ret = GetUserByName(username, &ui, NULL, &userindex)) != PROFILE_RETURN_ALLOK)
                        return ret;
        }
        else {
                // use userindex
                if(!GetUInfo(userindex, &ui))
                        return PROFILE_RETURN_DB_ERROR;
        }

        msg = *tomessages = *frommessages = NULL;

        // if we really need read to messages
        if(tocount == NULL) {
                // all messages
                toread = ui.persmescnt;
        }
        else {
                if(*tocount == 0) {
                        // only new messages
                        toread = ui.persmescnt - ui.readpersmescnt;
                }
                else {
                        // selected count
                        if(*tocount > ui.persmescnt)
                                *tocount = ui.persmescnt;
                        toread = *tocount;
                }
        }
        if(!toread) {
                *tomessages = NULL;
                goto skip_to_msg_read;
        }

        msg = (SPersonalMessage*)malloc(toread*sizeof(SPersonalMessage));

        if((fp = wcfopen(F_PROF_PERSMSG, FILE_ACCESS_MODES_R)) == NULL)
                return PROFILE_RETURN_DB_ERROR;
        curpos = ui.persmsg;
        for(i = 0; i < toread; i++) {
                if(wcfseek(fp, curpos, SEEK_SET) != 0)
                        goto PostPersMsg_Error;
                if(!fCheckedRead(&(msg[i]), sizeof(SPersonalMessage), fp))
                        goto PostPersMsg_Error;
                curpos = msg[i].Prev;
                // this situation should not happen, but...
                if(curpos == 0xffffffff) break;
        }
        msg[toread - 1].Prev = 0xffffffff;        // last message mark
        wcfclose(fp);
        *tomessages = msg;
        msg = NULL;

skip_to_msg_read:

        // if we really need read from messages
        if(fromcount == NULL) {
                // all messages
                fromread = ui.postedmescnt;
        }
        else {
                // selected count
                if(*fromcount > ui.postedmescnt)
                        *fromcount = ui.postedmescnt;
                fromread = *fromcount;
        }
        if(!fromread) {
                *frommessages = NULL;
                return PROFILE_RETURN_ALLOK;
        }

        msg = (SPersonalMessage*)malloc(fromread*sizeof(SPersonalMessage));

        if((fp = wcfopen(F_PROF_PERSMSG, FILE_ACCESS_MODES_R)) == NULL)
                return PROFILE_RETURN_DB_ERROR;
        curpos = ui.postedpersmsg;
        for(i = 0; i < fromread; i++) {
                if(wcfseek(fp, curpos, SEEK_SET) != 0)
                        goto PostPersMsg_Error;
                if(!fCheckedRead(&(msg[i]), sizeof(SPersonalMessage), fp))
                        goto PostPersMsg_Error;
                curpos = msg[i].PosterPrev;
                msg[i].Prev = 0;
                // this situation should not happen, but...
                if(curpos == 0xffffffff) {
                        break;
                }
        }
        msg[i].Prev = 0xffffffff;        // last message mark
        wcfclose(fp);
        *frommessages = msg;

        return PROFILE_RETURN_ALLOK;

PostPersMsg_Error:
        if(msg) free(msg);
        if(*tomessages){
                free(*tomessages);
                *tomessages = NULL;
        }
        if(*frommessages){
                free(*frommessages);
                *frommessages = NULL;
        }
        wcfclose(fp);
        return PROFILE_RETURN_DB_ERROR;
}

int CProfiles::ReadPersonalMessagesByDate(char *username, DWORD userindex,
                                                                                  SPersonalMessage **tomessages, time_t todate,
                                                                                  SPersonalMessage **frommessages, time_t fromdate)
{
        SProfile_UserInfo ui;
        int ret;
        WCFILE *fp;
        SPersonalMessage *msg;
        DWORD i, curpos;

        if(username != NULL && strcmp(username, "") != 0) {
                // use username
                if((ret = GetUserByName(username, &ui, NULL, &userindex)) != PROFILE_RETURN_ALLOK)
                        return ret;
        }
        else {
                // use userindex
                if(!GetUInfo(userindex, &ui))
                        return PROFILE_RETURN_DB_ERROR;
        }

        msg = *tomessages = *frommessages = NULL;

        // if we really need read "to" messages ?
        if(!todate) {
                *tomessages = NULL;
                goto skip_to_msg_read;
        }

        if((fp = wcfopen(F_PROF_PERSMSG, FILE_ACCESS_MODES_R)) == NULL)
                return PROFILE_RETURN_DB_ERROR;
        msg = (SPersonalMessage*)malloc(sizeof(SPersonalMessage));
        if(!msg) {
                wcfclose(fp);
                return PROFILE_RETURN_UNKNOWN_ERROR;
        }
        curpos = ui.persmsg;
        i = 0;
        if(curpos != 0xffffffff) {
                for(;;) {
                        if(wcfseek(fp, curpos, SEEK_SET) != 0)
                                goto PostPersMsg_Error;
                        if(!fCheckedRead(&(msg[i]), sizeof(SPersonalMessage), fp))
                                goto PostPersMsg_Error;
                        curpos = msg[i].Prev;
                        if(msg[i].Date < todate)
                                break;
                        i++;
                        if(curpos == 0xffffffff) break;
                        msg = (SPersonalMessage*)realloc(msg, (i+1)*sizeof(SPersonalMessage));
                }
        }
        if(!i) {
                free(msg);
                msg = NULL;
        }
        else msg[i-1].Prev = 0xffffffff;        // last message mark
        wcfclose(fp);
        *tomessages = msg;
        msg = NULL;

skip_to_msg_read:

        // if we really need read from messages
        if(!fromdate) {
                *frommessages = NULL;
                return PROFILE_RETURN_ALLOK;
        }

        if((fp = wcfopen(F_PROF_PERSMSG, FILE_ACCESS_MODES_R)) == NULL)
                return PROFILE_RETURN_DB_ERROR;
        msg = (SPersonalMessage*)malloc(sizeof(SPersonalMessage));
        if(!msg) {
                wcfclose(fp);
                return PROFILE_RETURN_UNKNOWN_ERROR;
        }
        curpos = ui.postedpersmsg;
        i = 0;
        if(curpos != 0xffffffff) {
                for(;;) {
                        if(wcfseek(fp, curpos, SEEK_SET) != 0)
                                goto PostPersMsg_Error;
                        if(!fCheckedRead(&(msg[i]), sizeof(SPersonalMessage), fp))
                                goto PostPersMsg_Error;
                        curpos = msg[i].PosterPrev;
                        msg[i].Prev = 0;
                        if(msg[i].Date < fromdate)
                                break;
                        i++;
                        if(curpos == 0xffffffff) break;
                        msg = (SPersonalMessage*)realloc(msg, (i+1)*sizeof(SPersonalMessage));
                }
        }
        if(!i) {
                free(msg);
                msg = NULL;
        }
        else msg[i - 1].Prev = 0xffffffff;        // last message mark
        wcfclose(fp);
        *frommessages = msg;

        return PROFILE_RETURN_ALLOK;

PostPersMsg_Error:
        if(msg) free(msg);
        if(*tomessages){
                free(*tomessages);
                *tomessages = NULL;
        }
        if(*frommessages){
                free(*frommessages);
                *frommessages = NULL;
        }
        wcfclose(fp);
        return PROFILE_RETURN_DB_ERROR;
}

int CProfiles::CheckandAddFavsList(DWORD userindex, DWORD msgindex, int doadd)
{
        int i, empty=0, fl=0;
        if( (msgindex == 0) || (userindex == 0)) return PROFILE_RETURN_INVALID_FORMAT;
        SProfile_UserInfo ui;
        if(!GetUInfo(userindex, &ui)) return PROFILE_RETURN_DB_ERROR;
        for( i =0; i < PROFILES_FAV_THREADS_COUNT; i++){
                if( ui.favs[i] == msgindex) return PROFILE_RETURN_ALREADY_EXIST;
                if(ui.favs[i] == 0) { if(!empty) empty =i+1;}
                else {if(ui.favs[i] < msgindex) fl=i+1;}
        }
        if( empty ){
                if( doadd == 0) return PROFILE_RETURN_ALLOK;
                DWORD prev=0;
                for( i =0; i < PROFILES_FAV_THREADS_COUNT; i++){
                        if  (i > fl && i > empty) {
                                prev=ui.favs[PROFILES_FAV_THREADS_COUNT-1];
                                break;
                        }
                        if(i) ui.favs[i-1] = prev;
                        if(  (i - empty + 2 > 0)   &&  (i - fl + 1 < 0) ) {
                                prev=ui.favs[i+1];
                                continue;
                        }
                        if( ((i - fl + 1 > 0) && (i - empty < 0)) || ((i -fl +1 == 0) && (i - empty + 2 > 0)) ) {
                                prev=msgindex;
                                msgindex=ui.favs[i];
                                continue;
                        }
                        prev=ui.favs[i];
                }
                ui.favs[PROFILES_FAV_THREADS_COUNT-1]=prev;
                if(SetUInfo(userindex, &ui)) return PROFILE_RETURN_ALLOK;
                return PROFILE_RETURN_DB_ERROR;
        }
        return PROFILE_RETURN_UNKNOWN_ERROR;
}

int CProfiles::DelFavsList(DWORD userindex, DWORD msgindex)
{
        DWORD i;
        if( (msgindex == 0) || (userindex == 0)) return PROFILE_RETURN_INVALID_FORMAT;
        SProfile_UserInfo ui;
        if(!GetUInfo(userindex, &ui)) return PROFILE_RETURN_DB_ERROR;

        for( i =0; i < PROFILES_FAV_THREADS_COUNT; i++){
                //check if msg is saved here and deleting if
                if(ui.favs[i] == msgindex) {
                        ui.favs[i] = 0;
                        if(SetUInfo(userindex, &ui)) return PROFILE_RETURN_ALLOK;
                        return PROFILE_RETURN_DB_ERROR;
                }
        }
        return PROFILE_RETURN_UNKNOWN_ERROR;
}
