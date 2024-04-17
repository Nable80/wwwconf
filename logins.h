/***************************************************************************
                          login.cpp  -  log in/out support
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef LOGINS_H_INCLUDED
#define LOGINS_H_INCLUDED

#include "basetypes.h"
#include "profiles.h"

// TODO: get rid of this chunking and read the whole authuser.dat with just one call
#define SEQUENCE_READ_COUNT 1000
#define SEQUENCE_LIVE_TIME USER_SESSION_LIVE_TIME

struct SSavedAuthSeq {
        DWORD ID[2];                //        ID of session (8 bytes)
        DWORD UniqID;                //        UniqID of user
                                                //        and also high bit of it is flag
#define SEQUENCE_IP_CHECK_DISABLED        0xf0000000
        DWORD IP;                        //        IP address of session
        DWORD SIndex;                //        Index in profindex file
        time_t ExpireDate;        //        Expiration date of the session
};
static_assert(sizeof(SSavedAuthSeq) == 24, "sizeof(SSavedAuthSeq) must be 24 (0x18)");

struct SAuthUserSeq {
        /* sequence number */
        DWORD ID[2];
        /* index of SProfile_UserInfo structure */
        DWORD SIndex;
        /* User specific parameters */
        DWORD right;
        /* uniq user ID */
        DWORD UniqID;
        // These two fields are not used now.
        // security level of message and header
        BYTE sec;
        BYTE sechdr;
        // Expiration date of the session
        time_t ExpireDate;
};
static_assert(sizeof(SAuthUserSeq) == 28, "sizeof(SAuthUserSeq) must be 28 (0x1C)");

/* contain information about logged in user or
 * about default user
 */
class CUserLogin {
public:
        CProfiles uprof;
        DWORD errnum;
        SProfile_UserInfo ui;
        SProfile_FullUserInfo fui;
        SAuthUserSeq LU;

        SSavedAuthSeq SEQ;

        /* constructor opens default session */
        CUserLogin();
        DWORD OpenSession(const char *uname, const char *passw, SProfile_FullUserInfo *ofui, DWORD lIP, DWORD IPCheckD);
        DWORD CheckSession(DWORD seq[2], DWORD lIP, DWORD Uid);
        DWORD GetUserInfoStruct(DWORD seq[2]);
        DWORD ForceCloseSessionForUser(DWORD UniqID);
        DWORD ForceCloseSessionBySeq(DWORD seq[2]);
        DWORD CloseSession(DWORD seq[2]);
        DWORD GenerateListSessionForUser(char ***buflist, DWORD *sc, DWORD Uid);
};

#endif

