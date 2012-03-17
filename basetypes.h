/***************************************************************************
                          basetypes.h  -  base types and definitions
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef BASETYPES_H_INCLUDED
#define BASETYPES_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <errno.h>
#include <ctype.h>


#define UNUSED(x) x __attribute__((__unused__))

const char *GetBoardUrl();

// VERSION 
#define VERSION                "2.0 PRE BETA5"


/****************** Log&Debug ******************/
// can be 0 or 1 
// 0 - disabled [Recomended]
// 1 - enabled
#define _DEBUG_        1
//
// can be 0, 1, 2
// 0 - fully disable, even critical errors
// 1 - disable, but critical error [Recommended]
// 2 - fully enable (information, critical errors, etc.)
#define ENABLE_LOG        2
//
// should file and string of each global error be showed to the user (useful for debug)
#define ERROR_ON_SCREEN        1
/***********************************************/

/*************************** language and locale ***************************/
#define LANGUAGE_LOCALE "ru_RU.CP1251"                // Unix

// should we set up locale during initialization
#define USE_LOCALE 0

#define RT_REDIRECT 1

// spelling check before post message
#define SPELLING_CHECK 1

// banned ip check before post message
#define BANNED_CHECK 1

// if message body not have be empty
#define MSG_REQURED_BODY 0

// enable IP to DNS name reslove Host Addresses
#define IP_TO_HOSTNAME_RESOLVE 1

#define DATETIME_DEFAULT_TIMEZONE 4

// show or not host names in messages
#define SHOW_HOST_NAME 1

// allow remark new messages
// 1 = mark with +
// 2 = mark will be linked to the next new message
#define ALLOW_MARK_NEW_MESSAGES        2

// should wwwconf change header of page on different pages
#define STABLE_TITLE 0

// antispam function control
#define ALLOW_ANTISPAM 1
// post time limit (in seconds), reason to decide that it's flood
// *** processed ONLY if antispam function active ***
#define POST_TIME_LIMIT 10

// refresh to posted message time
#define AUTO_REFRESH_TIME 3

/*****************************************************************************/
// topic support
#define TOPICS_SYSTEM_SUPPORT        1

// should we allow posting messages as unregistred when the same registred user exists
#define POST_ALLOW_UNDER_SAME_NICK 0

//**************************************
// support for user profiles
// *** if this feature is turned off NOBODY security byte and rigth will be applied to all users ***
#define USER_PROFILES_SUPPORT        1

#define USER_ALT_NICK_SPELLING_SUPPORT 1

// default session live time (for logged in user)
#define USER_SESSION_LIVE_TIME        360000

// user personal message support, allows to use conference as private messenger
#define USER_PERSONAL_MESSAGE_SUPPORT 1
//**************************************

// user favourites support, allow save messages in profile
// 1 = allow save any message to favourites
// 2 = allow save only parent message in thread
#define USER_FAVOURITES_SUPPORT 2

// support for global announces
#define GLOBAL_ANNOUNCES_SUPPORT        1

// support for user activity internal counters
#define ACTIVITY_LOGGING_SUPPORT        1

//        top and bottom banner located in file or defined statically
#define USE_HTML_BOTTOMBANNER        1
#define USE_HTML_TOPBANNER                1

//delete unused accounts 
#define CLEANUP_IDLE_USERS 1

//        count of messages on one page of search
#define SEARCH_MES_PER_PAGE_COUNT        100

/********************* DEFAULT USER AND MESSAGE PARAMETERS *******************/

/******** message database structures length *********/
#define MESSAGE_HEADER_LENGTH        100
#define AUTHOR_NAME_LENGTH                30
#define HOST_NAME_LENGTH                60

/************* default not logged users parameters ***************/
#define DEFAULT_NOBODY_RIGHT                USERRIGHT_CREATE_MESSAGE | USERRIGHT_VIEW_MESSAGE | USERRIGHT_CREATE_MESSAGE_THREAD | USERRIGHT_PROFILE_CREATE

/******** default user creation parameters (logged users) ********/
#define DEFAULT_USER_RIGHT                                USERRIGHT_CREATE_MESSAGE | USERRIGHT_VIEW_MESSAGE | USERRIGHT_CREATE_MESSAGE_THREAD | USERRIGHT_PROFILE_MODIFY | USERRIGHT_PROFILE_CREATE
#define USER_DEFAULT_PROFILE_CREATION_FLAGS PROFILES_FLAG_VISIBLE_EMAIL | PROFILES_FLAG_VIEW_SETTINGS

/******** default admin creation parameters (moderators) *********/
#define DEFAULT_ADMIN_RIGHT                        USERRIGHT_SUPERUSER

/******************** default "own settings" *********************/
// which will be used too if browser does not support cookie
#define CONFIGURE_SETTING_DEFAULT_lsel       2  // 1 - by time
#define CONFIGURE_SETTING_DEFAULT_tv         8  // 12 hours
#define CONFIGURE_SETTING_DEFAULT_tt         1  // hours
#define CONFIGURE_SETTING_DEFAULT_tc         100
#define CONFIGURE_SETTING_DEFAULT_ss         SHOW_MESSAGE_STYLE_HRON_FORWARD
#define CONFIGURE_SETTING_DEFAULT_dsm        CONFIGURE_plu | CONFIGURE_clr
#define CONFIGURE_SETTING_DEFAULT_topics     0x7FFFF
#define CONFIGURE_SETTING_DEFAULT_toverride  0  // own settings

#define CONFIGURE_SETTING_MAX_tc     10000
#define CONFIGURE_SETTING_MAX_hours  1440u
/********************** common params **********************/
#define MY_CGI_URL        ""
#define MY_HOST_URL        ""

#define BOARD_PIC_URL        "pic/"                                                // smile dir (HTTP path to pic with smiles)
                                                                                                        //
                                                                                                        // NOTE: it's not absolute path on server
                                                                                                        // it's only HTTP path

#define HTTP_REFERER_CHECK        0        
// this string should be in HTTP_REFERER to pass trough the test
#define ALLOWED_HTTP_REFERER "rt.mipt.ru"

// MAIL

// WC_TYPE 
// 1 - via smtp  ( localhost or remote server)
// 2 - via command line (sendmail in unix like system)

#define MA_TYPE                        1

#define MA_READURL                "http://board.rt.mipt.ru/"
#define MA_FROM                        "automailer-noreply@rt.mipt.ru"
// there should be smtp server ip or path to sendmail-like program
#define MA_SENDER                 "localhost"
#define ADMIN_MAIL        "bbsadmin@rt.mipt.ru"

#define COOKIE_NAME_STRING                        "RTBB="
#define COOKIE_SESSION_NAME                        "SessionRT="
#define COOKIE_EXPIRATION_DATE                "Fri, 31-Dec-2019 00:00:00 GMT;"
#define COOKIE_SERVER_PATH                        "/;"

// the maximum size for top and bottom files
#define MAX_HTML_FILE_SIZE 65535

/******************* perfomace parameters *******************/

// optimize index building for short index (about 300-500 messages in index)
// it will sligtly improove perfomance on HDD with high seek time
#define SHORT_INDEX_OPTIMIZATION        1

// count of message table entries for caching working with it
// this value is limited to 32000
#define READ_MESSAGE_TABLE  2000

// count of message index entries to cache working with it
// this value limited to 32000
#define READ_MESSAGE_HEADER 2*65535/sizeof(SMessage)

#define XML_BODY_MAX 1024

///////////////////////////////////////////////////////////////////////////////
////////////////////        Platform dependent parameters
///////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef unsigned short WORD;
typedef unsigned long DWORD;

#define WCFILE FILE

#define wcfopen fopen
#define wcfclose fclose
#define wcfseek fseek
#define wcfeof feof
#define wcfread fread
#define wcfwrite fwrite
#define wcftell ftell
#define wcfflush fflush

#define lock_file(a)        {fflush(a);flock(fileno(a), LOCK_EX);}
#define unlock_file(a)        {fflush(a);flock(fileno(a), LOCK_UN);}

//string case-insensitive compare
#define strcmpi strcasecmp



///////////////////////////////////////////////////////////////////////////////
/////////////////////        Platform independent MACRO definitions
///////////////////////////////////////////////////////////////////////////////

#define FILE_ACCESS_MODES_RW        "rb+"        // read-write mode
#define        FILE_ACCESS_MODES_R                "rb"        // read mode
#define FILE_ACCESS_MODES_CW        "wb+"        // create new (overwrite)

//        Environment params name
#define QUERY_STRING        "QUERY_STRING"
#define REQUEST_URI                "REQUEST_URI"
#define REMOTE_ADDR                "REMOTE_ADDR"

//        HTML checked or/and selected
#define RADIO_CHECKED                " CHECKED"
#define LISTBOX_SELECTED        " SELECTED"

#define fCheckedRead(buffer, size, f)        (wcfread(buffer, 1, size, f) == size)
#define fCheckedWrite(buffer, size, f)        (wcfwrite(buffer, 1, size, f) == size)
typedef unsigned char BYTE;
#define M_IN(x,a,b) (((x)>=(a))&&((x)<=(b)))


///////////////////////////////////////////////////////////////////////////////
/////////////////////        Internal parameters
///////////////////////////////////////////////////////////////////////////////

/*---------------------- DIRECTORIES ----------------------*/
#define DIR_MAINDATA        "data/"
#define DIR_MESSAGES        DIR_MAINDATA "messages/"                        // messages dir, for message database only
#define DIR_SEARCHER        DIR_MAINDATA "searcher/"                        // searcher dir, for search index only
#define DIR_PROFILES        DIR_MAINDATA "profiles/"                        // profiles dir, profile database only
#define DIR_SETTINGS        DIR_MAINDATA "settings/"                        // settings dir, for forum settings
#define DIR_INTERNALS        DIR_MAINDATA "internals/"                        // internal settings that will be autocreated
#define DIR_PROF_PIC        DIR_PROFILES "pic/"        // profile pictures dir

/*-------------------- PROFILES DATABASE ------------------*/
#define F_PROF_INDEX        DIR_PROFILES "profindex.idx"        // profile login indexes
#define F_PROF_NINDEX        DIR_PROFILES "profindex.db"                // profile indexes
#define F_PROF_FREENIDX        DIR_PROFILES "profifree.db"                // profile free nindex indexes
#define F_PROF_BODY                DIR_PROFILES "profbody.db"                // profile bodies (full user info)
#define F_PROF_FREEBODY        DIR_PROFILES "profbfree.db"                // profile free body indexes
#define F_PROF_PERSMSG        DIR_PROFILES "profpers.db"                // profile personal messages index
#define F_PROF_ALTNICK        DIR_PROFILES "profcnicks.db"        // profile alternative nick spelling

/*------------------- CONFIGURATION FILES -----------------*/
#define F_CONFIG                "wwwconf.conf"                // config file -- NOT YET SUPPORTED

/*------------------ MESSAGE DATABASE FILES ---------------*/
#define F_MSGINDEX                DIR_MESSAGES "index.msg"            // message headers
#define F_MSGBODY                DIR_MESSAGES "messages.msg"         // message bodies
#define F_INDEX                        DIR_MESSAGES "ra_index.msg"         // random access index file (hronological indexes)
#define F_VINDEX                DIR_MESSAGES "vra_index.msg"        // virtual index file
#define F_FREEMBODY                DIR_MESSAGES "freemess.msg"                // free spaces in message bodies
#define F_FREEINDEX                DIR_MESSAGES "freeindex.msg"        // free spaces in message headers
#define F_GLOBAL_ANN        DIR_MESSAGES "globalann.msg"        // global announces

/*------------------ SEARCH INDEX FILES -------------------*/
#define F_SEARCH_INDEX                DIR_SEARCHER "messearch.idx"        // searcher message index file
#define F_SEARCH_DB                        DIR_SEARCHER "messearch.db"                // searcher message index file
#define F_SEARCH_LASTINDEX        DIR_SEARCHER "lastindex"                // database id and last indexed message

/*---------------- SETTINGS FILES --------------*/
#define F_BANNEDIP                DIR_SETTINGS "banned.txt"                // list of banned IP
#define F_BADWORDS                DIR_SETTINGS "badwords.txt"                // list of words, witch is resstricted message headers and bodies

/*---------------- INTERNAL SUPPORT FILES --------------*/
#define F_ANTISPAM                DIR_INTERNALS "antispam.dat"                // antispam system file
#define F_AUTHSEQ                DIR_INTERNALS "authuser.dat"                // currently authorized users (sessions)
#define F_ACTIVITYLOG1        DIR_INTERNALS "activitylog1.dat"        // user activity log file 1
#define F_ACTIVITYLOG2        DIR_INTERNALS "activitylog2.dat"        // user activity log file 2
#define F_ACTIVITYARCH        DIR_INTERNALS "activityarch.dat"        // user activity archive

/*---------------------- LOG FILE -------------------------*/
#define LOG_FILE                DIR_MAINDATA "wwwconf.log"                // log file

#define COOKIE_MAX_LENGTH                        400

#define MAX_STRING 255
/* maximal parameters string and message length */
#define MAX_PARAMETERS_STRING 65535

#define XML_INDEX_MAXLEN 100

#define DATE_PRINT_STYLE                                         1
#define THREAD_PRINT_STYLE                                        2
#define SHOW_MESSAGE_STYLE_HRON_FORWARD                1
#define SHOW_MESSAGE_STYLE_HRON_BACKWARD        2

#define PRINT_FORWARD        1
#define PRINT_BACKWARD        0
#define GO_FORWARD                1
#define GO_BACKWARD                0

/********** bit masks **********/
/********** Message flags ****************/
#define MESSAGE_HAVE_PICTURE                0x0001
#define MESSAGE_HAVE_URL                    0x0002
#define MESSAGE_HAVE_BODY                   0x0004
#define MESSAGE_IS_CLOSED                   0x0008
#define MESSAGE_IS_INVISIBLE                0x0010
#define MESSAGE_ENABLED_TAGS                0x0020
#define MESSAGE_ENABLED_SMILES              0x0040        // affects only to HEADER of msg
#define MESSAGE_MAIL_NOTIFIATION            0x0080
#define MESSAGE_COLLAPSED_THREAD            0x0100        // collapsed thread
#define MESSAGE_WAS_SIGNED                  0x0200        // message have signature
#define MESSAGE_ENABLED_HTML                0x0400        // message in html format
#define MESSAGE_HAVE_TEX                    0x0800
#define MESSAGE_HAVE_TUB                    0x1000


/* for printhtmlmessage_in_index proc */
#define MESSAGE_INDEX_PRINT_ITS_URL                0x0001
#define MESSAGE_INDEX_DISABLE_ROLLED        0x0002
#define MESSAGE_INDEX_PRINT_BLANK_URL        0x0004

/* for user right - all modify/close/roll/delete modes affect ONLY on own posts! */

#define USERRIGHT_COUNT                14

#define USERRIGHT_SUPERUSER                                0x0001        // allow ALL rigth (skip all right test)
#define USERRIGHT_VIEW_MESSAGE                        0x0002        // allow view messages in conference
#define USERRIGHT_MODIFY_MESSAGE                0x0004        // modify (change) own messages
#define USERRIGHT_CLOSE_MESSAGE                        0x0008        // close thread right
#define USERRIGHT_OPEN_MESSAGE                        0x0010        // open thread right (form closed)
#define USERRIGHT_CREATE_MESSAGE                0x0020        // create message (reply)
#define USERRIGHT_CREATE_MESSAGE_THREAD        0x0040        // create message (new thread)
#define USERRIGHT_ALLOW_HTML                        0x0080        // allow HTML right
#define USERRIGHT_PROFILE_MODIFY                0x0100        // allow modify, delete own profile
#define USERRIGHT_PROFILE_CREATE                0x0200        // allow create new profiles
#define USERRIGHT_ROLL_MESSAGE                        0x0400        // roll thread right
#define USERRIGHT_UNROLL_MESSAGE                0x0800        // unroll thread right
#define USERRIGHT_POST_GLOBAL_ANNOUNCE        0x1000        // post global conference announce
#define USERRIGHT_ALT_DISPLAY_NAME                0x2000        // alternative display name


/* configure view */
#define CONFIGURE_dsm        0x000001
#define CONFIGURE_clr        0x000002
#define CONFIGURE_bot        0x000004
#define CONFIGURE_onh        0x000008
#define CONFIGURE_plu        0x000010
#define CONFIGURE_host       0x000020
#define CONFIGURE_nalt       0x000040
#define CONFIGURE_dsig       0x000080
#define CONFIGURE_shrp       0x000100

/* for virtual indexes */
#define NO_MESSAGE_CODE        0xFFFFFFFF

/*        Common message structure */
struct SMessage
{
        /* common parameters */
        char MessageHeader[MESSAGE_HEADER_LENGTH];
        char AuthorName[AUTHOR_NAME_LENGTH];
        char HostName[HOST_NAME_LENGTH];

        /* IP Address of poster */
        DWORD IPAddr;

        /* unique user ID or 0, if anonymous posts */
        DWORD UniqUserID;

        /* These two fields are not used now. */
        /* security level of message (for tags, etc.) */
        /* body security */
        BYTE Security;
        /* header security */
        BYTE SecHeader;

        /* flag of message : HAVE_PIC, CLOSED, etc. */
        DWORD Flag;

        /* creation time */
        time_t Date;
        /* modifying  date */
        time_t MDate;

        /* message level in tree */
        WORD Level;

        /* topic flags */
        DWORD Topics;

        /* request counter of this message */
        WORD Readed;

        /* index of main thread message */
        DWORD ParentThread;

        /* virtual index of message */
        DWORD ViIndex;

        /* index of message body */
        DWORD MIndex;
        /* size of message body */
        DWORD msize;
        
        SMessage()
        : IPAddr(0), UniqUserID(0), Flag(0),
                Date(0), MDate(0), Level(0), Topics(0), Readed(0),
                ParentThread(0), ViIndex(0), MIndex(0), msize(0)
        {
                MessageHeader[0] = '\0';
                AuthorName[0] = '\0';
                HostName[0] = '\0';

        }
};


///////////////////////////////////////////////////////////////////////////////
/////////////////////        Subroutine definitions
///////////////////////////////////////////////////////////////////////////////

char* toupperstr(char *s);

#endif // of BASETYPES_H_INCLUDED
