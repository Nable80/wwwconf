/***************************************************************************
                          login.cpp  -  log in/out support
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "security.h"
#include "logins.h"
#include "error.h"

#define unlock_and_logins_io_error() {unlock_file(f); wcfclose(f); return 0;}
#define logins_lock_file() {lock_file(f);}
#define logins_unlock_file() {unlock_file(f);}

/* create authorization sequence with ttl - Time To Live for user *ui
 * and store it to ui.ID
 * return 1 if successfull, otherwise zero returned
 */
DWORD OpenAuthSequence(SSavedAuthSeq *ui)
{
        SSavedAuthSeq *buf;
        WCFILE *f;
        int el;
        DWORD id, id1, rr = 0, i, ii;

        /* set creation time */
        time_t tn = time(NULL);

        if((f = wcfopen(F_AUTHSEQ, FILE_ACCESS_MODES_RW)) == NULL) {
                // if file doesn't exist - create it
                if((f = wcfopen(F_AUTHSEQ, FILE_ACCESS_MODES_CW)) == NULL) {
                        return 0;
                }
                else {
                        wcfclose(f);
                        if((f = wcfopen(F_AUTHSEQ, FILE_ACCESS_MODES_RW)) == NULL) {
                                return 0;
                        }
                }
        }
        buf = (SSavedAuthSeq*)malloc(sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT);

        /******** lock f ********/
        logins_lock_file();

        ii = 0;
        while(!wcfeof(f)) {
                if((rr = wcfread(((char*)buf + ii*sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT), 1, sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT, f)) !=
                        sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT) {
                        if((rr%sizeof(SSavedAuthSeq)) != 0) {
                                free(buf);
                                unlock_and_logins_io_error();
                        }
                }
                ii++;
                buf = (SSavedAuthSeq*)realloc(buf, (ii+1)*sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT);
        }

        // Generate random sequence for user session
        // Standard mersenne_twister_engine seeded with rand_dev()
        std::random_device rand_dev;
        std::mt19937 rand_gen(rand_dev());
        std::uniform_int_distribution<DWORD> dword_distrib(0, UINT32_MAX);
L_Try:
//        print2log("->>> %d", ((ii-1)*SEQUENCE_READ_COUNT + (rr+1)/sizeof(SAuthUserSeq)));
        id = dword_distrib(rand_gen);
        id1 = dword_distrib(rand_gen);
        el = -1;
        for(i = 0; i < ((ii-1)*SEQUENCE_READ_COUNT + (rr+1)/sizeof(SSavedAuthSeq)); i++) {
                // regenerate id/id1 if the same sequence already exists
                if(buf[i].ID[0] == id && buf[i].ID[1] == id1)
                        goto L_Try;
                if(buf[i].ExpireDate < tn) {
                        if(el < 0) el = i;
                }
        }

        free(buf);

        // Sequence was successfully generated, save it
        ui->ID[0] = id;
        ui->ID[1] = id1;
        ui->ExpireDate = time(NULL) + SEQUENCE_LIVE_TIME;

        if(el < 0) {
                if(wcfseek(f, 0, SEEK_END) < 0)
                        unlock_and_logins_io_error();
        }
        else {
                if(wcfseek(f, el*sizeof(SSavedAuthSeq), SEEK_SET) < 0)
                        unlock_and_logins_io_error();
        }

        if(!fCheckedWrite(ui, sizeof(SSavedAuthSeq), f))
                unlock_and_logins_io_error();

        logins_unlock_file();
        /********* unlock f *********/

        wcfclose(f);
        return 1;
}

/* close authorization sequence by Session id (=id) or userId (=Uid)
 * return 1 if successfull otherwise 0 returned
 */
int CloseAuthSequence(DWORD id[2], DWORD Uid)
{
        SSavedAuthSeq *buf = NULL;
        WCFILE *f;
        DWORD rr = 0, i, ii = 0;
        time_t tn = time(NULL);

        if(( (id[0] != 0 || id[1] != 0) && Uid != 0) || (id[0] == 0 && id[1] == 0 && Uid == 0))
                return 0;        // incorrect params
        
        if((f = wcfopen(F_AUTHSEQ, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;

        buf = (SSavedAuthSeq*)malloc(sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT);

        //******** lock f ********
        logins_lock_file();

        while(!wcfeof(f)) {
                if((rr = wcfread(((char*)buf + ii*sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT), 1, sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT, f)) !=
                        sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT)
                {
                        if((rr%sizeof(SSavedAuthSeq)) != 0) {
                                free(buf);
                                unlock_and_logins_io_error();
                        }
                }
                ii++;
                buf = (SSavedAuthSeq*)realloc(buf, (ii+1)*sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT);
        }

        int changed = 0;
        for(i = 0; i < ((ii-1)*SEQUENCE_READ_COUNT + (rr+1)/sizeof(SSavedAuthSeq)); i++)
        {
                if( buf[i].ExpireDate > tn && ((buf[i].ID[0] == id[0] && buf[i].ID[1] == id[1] && Uid == 0) ||
                        (buf[i].UniqID == Uid && id[0] == 0 && id[1] == 0)) )
                {
                        if(wcfseek(f, i*sizeof(SSavedAuthSeq), SEEK_SET) < 0) {
                                free(buf);
                                unlock_and_logins_io_error();
                        }

                        // set time as sequence expired == free sequence
                        buf[i].ExpireDate = time(NULL) - 1;

                        if(wcfwrite(&buf[i], 1, sizeof(SSavedAuthSeq), f) != sizeof(SSavedAuthSeq)) {
                                free(buf);
                                unlock_and_logins_io_error();
                        }
                        changed = 1;
                        if(id[0] != 0) break;
                }
        }

        free(buf);

        //******** unlock f ********
        logins_unlock_file();

        wcfclose(f);
        return changed;
}



/* list authorization sequences by userId (=Uid)
* return 1 if successfull otherwise 0 returned
*/
int GenerateListAuthSequence(char ***buflist, DWORD *sc, DWORD Uid)
{
        *buflist = NULL;
        *sc = 0;
        SSavedAuthSeq *buf;
        WCFILE *f;
        DWORD rr, i, cn=0;

        if((f = wcfopen(F_AUTHSEQ, FILE_ACCESS_MODES_R)) == NULL) return 0;
                                                                
        buf = (SSavedAuthSeq*)malloc(SEQUENCE_READ_COUNT*sizeof(SSavedAuthSeq));
        if(!(*buflist = (char**)malloc(sizeof(char**)))) return 0;

        /******** lock f ********/
        logins_lock_file();
                                                                
        do {
                rr = wcfread(buf, 1, sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT, f);

                if((rr%sizeof(SSavedAuthSeq)) != 0) {
                /* read error */
                        free(buf);
                        for(i = 0; i < cn; i++)  free((*buflist)[i]);
                        free(*buflist);
                        *buflist = NULL;
                        unlock_and_logins_io_error();
                }
                                                                                                                                                                                                                                                                                                                                
                rr = rr/sizeof(SSavedAuthSeq);
                for(i = 0; i < rr; i++) {
                        if( Uid==0 || (buf[i].UniqID & (~SEQUENCE_IP_CHECK_DISABLED)) == Uid ){
                                *buflist = (char**)realloc(*buflist, (cn+1)*sizeof(char**));
                                (*buflist)[cn] = (char*)malloc(5*sizeof(DWORD) + 1);
                                memcpy(((char*)((*buflist)[cn])), &buf[i].ExpireDate, sizeof(DWORD));
                                memcpy(((char*)((*buflist)[cn]))+4, &buf[i].IP, sizeof(DWORD));
                                memcpy(((char*)((*buflist)[cn]))+8, &buf[i].ID[0], sizeof(DWORD));
                                memcpy(((char*)((*buflist)[cn]))+12, &buf[i].ID[1], sizeof(DWORD));
                                memcpy(((char*)((*buflist)[cn]))+16, &buf[i].UniqID, sizeof(DWORD));
                                cn++;
                        }
                }
        } while(rr == SEQUENCE_READ_COUNT);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
        free(buf);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
        logins_unlock_file();
        /********* unlock f *********/
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
        wcfclose(f);
        *sc = cn;
        return 1;        
}


                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
/* check and update sequence 'id' with ttl - Time To Live
 * return 1 if successfull otherwise -1 if not found and 0 if i/o error
 */
int CheckAndUpdateAuthSequence(DWORD id[2], DWORD IP, SSavedAuthSeq *ui)
{
        SSavedAuthSeq *buf;
        WCFILE *f;
        DWORD rr, i, cn, fpos;
        
        if((f = wcfopen(F_AUTHSEQ, FILE_ACCESS_MODES_RW)) == NULL)
                return 0;
        buf = (SSavedAuthSeq*)malloc(sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT);

        /******** lock f ********/
        logins_lock_file();


        while(!wcfeof(f)) {
                fpos = wcftell(f);
                rr = wcfread(buf, 1, sizeof(SSavedAuthSeq)*SEQUENCE_READ_COUNT, f);

                if((rr%sizeof(SSavedAuthSeq)) != 0) {
                        /* read error */
                        free(buf);
                        unlock_and_logins_io_error();
                }
                cn = rr/sizeof(SSavedAuthSeq);

                time_t tn = time(NULL);

                for(i = 0; i < cn; i++) {
                        if( buf[i].ExpireDate > tn && buf[i].ID[0] == id[0] && buf[i].ID[1] == id[1] &&
                        ( (buf[i].UniqID & SEQUENCE_IP_CHECK_DISABLED) != 0 || buf[i].IP == IP || IP == 0 ) )
                        {
                                memcpy((void*)ui, &buf[i], sizeof(SSavedAuthSeq));

                                if(IP){

                                        if(wcfseek(f, fpos + i*sizeof(SSavedAuthSeq), SEEK_SET) != 0) {
                                                free(buf);
                                                unlock_and_logins_io_error();
                                        }

                                        buf[i].ExpireDate = time(NULL) + SEQUENCE_LIVE_TIME;
                                        if(!fCheckedWrite(&buf[i], sizeof(SSavedAuthSeq), f)) {
                                                free(buf);
                                                unlock_and_logins_io_error();
                                        }
                                }

                                free(buf);
                                
                                logins_unlock_file();
                                /********* unlock f *********/
                                
                                wcfclose(f);
                                return 1;
                        }
                }
        }

        logins_unlock_file();
        /********* unlock f *********/

        return -1;
}

/********************** CUserLogin **********************/
CUserLogin::CUserLogin()
{
        uprof = NULL;
        pui = NULL;
        pfui = NULL;
        LU.SIndex = 0xFFFFFFFF;
        LU.right = DEFAULT_NOBODY_RIGHT;
        LU.UniqID = 0;
        LU.ID[0] = 0;
        LU.ID[1] = 0;
}

CUserLogin::~CUserLogin()
{
        if(uprof != NULL)        delete uprof;
        if(pui != NULL)                free(pui);
        if(pfui != NULL)        free(pfui);
}

/* open user session and return 1 if successfull (all information will be stored
 * to LU, otherwise return 0
 */
DWORD CUserLogin::OpenSession(char *uname, char *passw, SProfile_FullUserInfo *Fui, DWORD lIP, DWORD IPCheckD)
{
        int cr;
        if(uprof == NULL) {
                uprof = new CProfiles();
                if(uprof->errnum != PROFILE_RETURN_ALLOK) {
#if ENABLE_LOG >= 1
                        print2log("call to CProfiles::CProfiles failed at CUserLogin::OpenSession(), line %d", __LINE__);
#endif
                        return 0;
                }
        }

        if(pui == NULL) pui = (SProfile_UserInfo*)malloc(sizeof(SProfile_UserInfo));
        if(pfui == NULL) pfui = (SProfile_FullUserInfo*)malloc(sizeof(SProfile_FullUserInfo));

        if((cr = uprof->GetUserByName(uname, pui, pfui, &LU.SIndex)) == PROFILE_RETURN_ALLOK &&
                strcmp(passw, pui->password) == 0)
        {
                /* prepare SUserInfo structure */
                LU.right = pui->right;
                LU.UniqID = pui->UniqID;

                /* modify SUPERUSER right */
                if(LU.right & USERRIGHT_SUPERUSER) {
                        LU.right = LU.right | USERRIGHT_VIEW_MESSAGE | USERRIGHT_CLOSE_MESSAGE |
                                USERRIGHT_MODIFY_MESSAGE | USERRIGHT_CREATE_MESSAGE | USERRIGHT_CREATE_MESSAGE_THREAD |
                                USERRIGHT_OPEN_MESSAGE | USERRIGHT_ALLOW_HTML | USERRIGHT_PROFILE_MODIFY |
                                USERRIGHT_PROFILE_CREATE | USERRIGHT_POST_GLOBAL_ANNOUNCE | USERRIGHT_ALT_DISPLAY_NAME;
                }

                // fill sequence info
                //SEQ.ID;                                        //        ID of session
                //SEQ.Reserved;                                //        Reserved place
                //SEQ.ExpireDate;                        //        Expiration date of the session
                SEQ.UniqID = pui->UniqID;        //        UniqID of user
                if(IPCheckD) SEQ.UniqID |= SEQUENCE_IP_CHECK_DISABLED;
                SEQ.IP = lIP;                                //        IP address of session
                SEQ.SIndex = LU.SIndex;                //        Index in profindex file

                /* open sequence for user */
                if(OpenAuthSequence(&SEQ) != 1) {
                        LU.SIndex = 0xFFFFFFFF;
                        LU.right = DEFAULT_NOBODY_RIGHT;
                        LU.UniqID = 0;
                        LU.ID[0] = 0;
                        LU.ID[1] = 0;
                        free(pui);
                        free(pfui);
                        pui = NULL;
                        pfui = NULL;
#if ENABLE_LOG >= 1
                        print2log("Call to OpenAuthSequence failed at CUserLogin::OpenSession(), line %d", __LINE__);
#endif
                        return 0;
                }
                else {
                        // save to LU
                        LU.ID[0] = SEQ.ID[0];
                        LU.ID[1] = SEQ.ID[1];
                        LU.ExpireDate = SEQ.ExpireDate;
                }

                // update IP and last login date
                pui->lastIP = lIP;
                pui->LoginDate = time(NULL);
                if(uprof->SetUInfo(LU.SIndex, pui) != 1) {
#if ENABLE_LOG >= 1
                        print2log("Call to CProfiles::SetUInfo failed at CUserLogin::OpenSession(), lined %d", __LINE__);
#endif
                }

                // copy Full user info if requred
                if(Fui != NULL) memcpy(Fui, pfui, sizeof(SProfile_FullUserInfo));
                return 1;
        }
        else {
#if ENABLE_LOG >= 1
                if(cr == PROFILE_RETURN_DB_ERROR)
                        print2log("Call to CProfiles::GetUserByName failed at CUserLogin::OpenSession(), line %d", __LINE__);
#endif
                free(pui);
                free(pfui);
                pui = NULL;
                pfui = NULL;
                /* invalid username/password */
                return 0;
        }
}

/* check sequence seq for activity, set LU to this sequence and 
 * return 1 if successfull, otherwise return 0
 
 lIP == 0 - checking existing seq and Uid == seq.UniqID without updating (!!!)
 Uid ==0 full checking with updating

 */
DWORD CUserLogin::CheckSession(DWORD seq[2], DWORD lIP, DWORD Uid)
{

        if(  Uid != 0  && lIP != 0) goto SessionErrorEnd;  //invalid calls


        if(CheckAndUpdateAuthSequence(seq, lIP, &SEQ) == 1) {
                if(lIP){

                        if(uprof == NULL) {
                                uprof = new CProfiles();
                                if(uprof->errnum != PROFILE_RETURN_ALLOK) {
#if ENABLE_LOG >= 1
                                        print2log("call to CProfiles::CProfiles failed at CUserLogin::CheckSession(), line %d", __LINE__);
#endif
                                        goto SessionError1;
                                }
                        }

                        if(pui == NULL) pui = (SProfile_UserInfo*)malloc(sizeof(SProfile_UserInfo));
                        if(pfui == NULL) pfui = (SProfile_FullUserInfo*)malloc(sizeof(SProfile_FullUserInfo));

                        if(uprof->GetUInfo(SEQ.SIndex, pui) != 1) {
#if ENABLE_LOG >= 1
                                print2log("call to CProfiles::GetUInfo failed at CUserLogin::CheckSession(), line %d" \
                                        " (maybe user has been deleted)", __LINE__);
#endif
                                goto SessionError2;
                        }


                        
                        if(uprof->GetFullInfo(pui->FullInfo_ID, pfui) != 1) {
#if ENABLE_LOG >= 1
                                print2log("call to CProfiles::GetFullInfo() failed at CUserLogin::CheckSession(), line %d"
                                        " (maybe user have been deleted)", __LINE__);
#endif
                                goto SessionError2;
                        }

                        // security check that session was open by this account
                        if(pfui->CreateDate + SEQUENCE_LIVE_TIME > SEQ.ExpireDate) {
                                print2log("warning! session is older than account \"%s\"", pui->username);
                                goto SessionError2;
                        }

                        // Update IP and last login date
                        pui->lastIP = lIP;
                        pui->LoginDate = time(NULL);
                        pui->RefreshCount++;

                        if(uprof->SetUInfo(SEQ.SIndex, pui) != 1) {
#if ENABLE_LOG >= 1
                                print2log("Call to CProfiles::SetUInfo failed at CUserLogin::CheckSession(), lined %d", __LINE__);
#endif
                                goto SessionError2;
                        }


                        // Pui + Pfui is ok
                        // profile updated
                        // Load to LU

                        LU.ID[0] = SEQ.ID[0];
                        LU.ID[1] = SEQ.ID[1];
                        LU.SIndex = SEQ.SIndex;

                        LU.UniqID = (SEQ.UniqID & (~SEQUENCE_IP_CHECK_DISABLED));
                        LU.ExpireDate = SEQ.ExpireDate;

                        // LOAD REAL RIGHTS !!!
                        LU.right = pui->right;

                        /* modify SUPERUSER right */
                        if(LU.right & USERRIGHT_SUPERUSER) {
                                LU.right = LU.right | USERRIGHT_VIEW_MESSAGE | USERRIGHT_CLOSE_MESSAGE |
                                        USERRIGHT_MODIFY_MESSAGE | USERRIGHT_CREATE_MESSAGE | USERRIGHT_CREATE_MESSAGE_THREAD |
                                        USERRIGHT_OPEN_MESSAGE | USERRIGHT_ALLOW_HTML | USERRIGHT_PROFILE_MODIFY |
                                        USERRIGHT_PROFILE_CREATE | USERRIGHT_POST_GLOBAL_ANNOUNCE | USERRIGHT_ALT_DISPLAY_NAME;
                        }
                }
                else {
                        //smb wants check own Uid's session
                        if( Uid !=0 && (SEQ.UniqID & (~SEQUENCE_IP_CHECK_DISABLED)) != Uid ) return 0;
                }

                return 1;
        }
        // seqence cannot be found

        goto SessionErrorEnd; 
        
SessionError2:

        if(CloseAuthSequence(seq, 0) == 0) {}
                                
        free(pui);
        free(pfui);
        pui = NULL;
        pfui = NULL;

SessionError1:

        delete uprof;
        uprof = NULL;

SessionErrorEnd:

        // init as no session
        LU.SIndex = 0xFFFFFFFF;
        LU.right = DEFAULT_NOBODY_RIGHT;
        LU.UniqID = 0;
        LU.ID[0] = 0;
        LU.ID[1] = 0;

        return 0;
}

/* close sequence seq
 * return 1 if successfull, 0 otherwise
 */
DWORD CUserLogin::CloseSession(DWORD seq[2])
{
        if(CloseAuthSequence(seq, 0) == 1) {
                LU.SIndex = 0xFFFFFFFF;
                LU.right = DEFAULT_NOBODY_RIGHT;
                LU.UniqID = 0;
                LU.ID[0] = 0;
                LU.ID[1] = 0;
                if(pui != NULL) {
                        free(pui);
                        pui = NULL;
                }
                if(pfui != NULL) {
                        free(pfui);
                        pfui = NULL;
                }
                return 1;
        }
        return 0;
}

/* force closing session if user such user logged in.
 * return 1 if successfull, 0 otherwise
 */
DWORD CUserLogin::ForceCloseSessionForUser(DWORD UniqID)
{
        DWORD x[2];
        memset(&x, 0, 8);
        if(CloseAuthSequence(x, UniqID) == 1) {
                return 1;
        }
        return 0;
}

DWORD CUserLogin::ForceCloseSessionBySeq(DWORD seq[2])
{

        if(CloseAuthSequence(seq, 0) == 1) {
                return 1;
        }
        return 0;
}

DWORD CUserLogin::GenerateListSessionForUser(char ***list, DWORD *scounter, DWORD Uniqid)
{
        if( GenerateListAuthSequence(list, scounter, Uniqid) == 1) return 1;
        return 0;
}        
