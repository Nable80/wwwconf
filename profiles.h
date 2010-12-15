/***************************************************************************
                          profiles.h  -  profiles support include
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef PROFILES_H_INCLUDED
#define PROFILES_H_INCLUDED

#include "basetypes.h"
#include "hashindex.h"

/* database length parameters */
#define PROFILES_MAX_PASSWORD_LENGTH			33	
#define PROFILES_MAX_ICQ_LEN					16
#define PROFILES_MAX_ALT_DISPLAY_NAME			90
#define PROFILES_FAV_THREADS_COUNT				20	//4*20 = 80 bytes real size
#define PROFILES_MAX_USERNAME_LENGTH			30
#define PROFILES_MAX_SIGNATURE_LENGTH			255
#define PROFILES_FULL_USERINFO_MAX_NAME			255
#define PROFILES_FULL_USERINFO_MAX_EMAIL		255
#define PROFILES_FULL_USERINFO_MAX_HOMEPAGE		70
#define PROFILES_FULL_USERINFO_MAX_SELECTEDUSR	185

#define PROFILES_MIN_PASSWORD_LENGTH			3

/* bit mask for user flags */
#define PROFILES_FLAG_HAVE_PICTURE			0x0001
#define PROFILES_FLAG_HAVE_SIGNATURE		0x0002
#define PROFILES_FLAG_INVISIBLE				0x0004
#define PROFILES_FLAG_VISIBLE_EMAIL			0x0008
#define PROFILES_FLAG_NOT_ACTIVATED			0x0010
#define PROFILES_FLAG_VIEW_SETTINGS			0x0020
#define PROFILES_FLAG_DISABLED				0x0040
#define PROFILES_FLAG_PERSMSGDISABLED		0x0080
#define PROFILES_FLAG_PERSMSGTOEMAIL		0x0100
#define PROFILES_FLAG_ALWAYS_EMAIL_ACKN	0x0200
#define PROFILES_FLAG_ALT_DISPLAY_NAME		0x0400
#define PROFILES_FLAG_APPLY_CONF_LOGIN		0x0800


/* profile function error codes */
#define PROFILE_RETURN_ALLOK			0
#define	PROFILE_RETURN_ALREADY_EXIST	1
#define PROFILE_RETURN_DB_ERROR			3
#define PROFILE_RETURN_INVALID_FORMAT	4
#define PROFILE_RETURN_INVALID_LOGIN	5
#define PROFILE_RETURN_INVALID_PASSWORD	6
#define PROFILE_RETURN_PASSWORD_SHORT	7
#define PROFILE_RETURN_UNKNOWN_ERROR	9

#define PROFILE_LOGIN_MIN_LENGTH		3

#define PROFILE_NO_NEXT_INDEX			0xFFFFFFFF

#define PROFILE_WASTED_FINFO_SIZE	sizeof(SProfile_FullUserInfo) - sizeof(char*)

#define PROFILE_PERSONAL_MESSAGE_LENGHT	385

struct SPersonalMessage {
	DWORD Prev;
	DWORD PosterPrev;
	char NameFrom[PROFILES_MAX_USERNAME_LENGTH];
	DWORD UIdFrom;
	char NameTo[PROFILES_MAX_USERNAME_LENGTH];
	DWORD UIdTo;
	time_t Date;
	char Msg[PROFILE_PERSONAL_MESSAGE_LENGHT];
};


#pragma pack(1)

struct SViewSettings
{
	WORD dsm;
	DWORD topics;
	WORD tv;
	WORD tc;
	unsigned char ss:3;
	unsigned char lsel:2;
	unsigned char tt:4;
	char tz:5;
	unsigned char resrvd:2;
};

/* Full user information */
struct SProfile_FullUserInfo {
	char FullName[PROFILES_FULL_USERINFO_MAX_NAME];
	char Email[PROFILES_FULL_USERINFO_MAX_EMAIL];
	char HomePage[PROFILES_FULL_USERINFO_MAX_HOMEPAGE];
	char SelectedUsers[PROFILES_FULL_USERINFO_MAX_SELECTEDUSR];
	char Signature[PROFILES_MAX_SIGNATURE_LENGTH];

	time_t CreateDate;

	// Here you can define additional parameters

	// do not modify following 2 lines
	DWORD size;
	char *AboutUser;
};

/* Structure which keep information about user in profiles database */
struct SProfile_UserInfo {
	/* Username, password */
	char username[PROFILES_MAX_USERNAME_LENGTH];
	char password[PROFILES_MAX_PASSWORD_LENGTH];
	/* view settings */
	SViewSettings vs;
	char icqnumber[PROFILES_MAX_ICQ_LEN];
	char altdisplayname[PROFILES_MAX_ALT_DISPLAY_NAME];
	DWORD favs[PROFILES_FAV_THREADS_COUNT];
	// count of refresh of this board
	DWORD RefreshCount;
	// recieved persmsg read count
	WORD readpersmescnt;
	// received persmsg current count
	WORD persmescnt;
	// posted persmsg count
	WORD postedmescnt;
	// personal message index (or 0xFFFFFFFF if does not exist)
	DWORD persmsg;
	// posted personal message index (or 0xFFFFFFFF)
	DWORD postedpersmsg;
	// user status
	BYTE Status;
	// Ip of last login
	DWORD lastIP;
	// security level for message header
	BYTE secheader;
	BYTE align1[3];
	// unique user identifier
	DWORD UniqID;
	// flags for user (ex: have picture... etc.)
	DWORD Flags;
	// security level of user (for message body)
	BYTE secur;
	BYTE align2[3];
	// right for user
	DWORD right;
	// number of posts to conference
	DWORD postcount;
	time_t LoginDate;
	// index in profile bodies file of structure with common user information
	DWORD FullInfo_ID;
};
#pragma pack(4)

class CProfiles {
protected:
	WCFILE *Fp_i,	// profiles index file
		*Fp_b;		// profiles body file

	int ReadFullInfo(DWORD idx, SProfile_FullUserInfo *FI);
	int WriteFullInfo(DWORD *idx, SProfile_FullUserInfo *FI);
	int DeleteFullInfo(DWORD idx);

	int GetSpaceforUInfo(DWORD *idx);
	int WriteUInfo(DWORD idx, SProfile_UserInfo *FI);
	int DeleteUInfo(DWORD idx);
	int ReadUInfo(DWORD idx, SProfile_UserInfo *FI);
public:
	DWORD errnum;
	CProfiles();
	~CProfiles();

	int AddNewUser(SProfile_UserInfo *newprf, SProfile_FullUserInfo *FullUI, DWORD *ui_index);
	int ModifyUser(SProfile_UserInfo *newprf, SProfile_FullUserInfo *FullUI, DWORD *ui_index);
	int DeleteUser(char *name);
	int GetUserByName(char *name, SProfile_UserInfo *ui, SProfile_FullUserInfo *Fui, DWORD *ui_index);

	// personal messages
	int PostPersonalMessage(char *username, DWORD userindex, char *message, char *from, DWORD userindexfrom);
	int ReadPersonalMessages(char *username, DWORD userindex,
			SPersonalMessage **tomessages, DWORD *tocount, SPersonalMessage **frommessages, DWORD *fromcount);

	int ReadPersonalMessagesByDate(char *username, DWORD userindex,
			SPersonalMessage **tomessages, time_t todate, SPersonalMessage **frommessages, time_t fromdate);

	int GenerateUserList(char ***buf, DWORD *cnt);

	int GetUsersCount(DWORD *uc);
	int GetUInfo(DWORD idx, SProfile_UserInfo *FI);
	int SetUInfo(DWORD idx, SProfile_UserInfo *FI);
	int GetFullInfo(DWORD idx, SProfile_FullUserInfo *FI);

	int CheckandAddFavsList(DWORD userindex, DWORD msgindex, int doadd);
	int DelFavsList(DWORD userindex, DWORD msgindex);
};

#endif
