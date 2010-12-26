/***************************************************************************
                          announces.h  -  board announces support header
                             -------------------
    begin                : Mon Feb 24 2003
    copyright            : (C) 2001 - 2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef ANNOUNCES_H_INCLUDED
#define ANNOUNCES_H_INCLUDED

#include "basetypes.h"
#include "profiles.h"

#define GLOBAL_ANNOUNCE_MAXSIZE 513

#define ANNOUNCES_RETURN_OK                        0
#define ANNOUNCES_RETURN_IO_ERROR        1
#define ANNOUNCES_RETURN_DB_ERROR        2
#define ANNOUNCES_RETURN_LOW_RES        3
#define ANNOUNCES_RETURN_NOT_FOUND        4

#define ANNOUNCES_UPDATE_OPT_USER        0x01
#define ANNOUNCES_UPDATE_OPT_TIME        0x02
#define ANNOUNCES_UPDATE_OPT_FLAGS        0x04
#define ANNOUNCES_UPDATE_OPT_TTL        0x08

#pragma pack(1)
struct SGlobalAnnounce {
        char Announce[GLOBAL_ANNOUNCE_MAXSIZE];

        char From[PROFILES_MAX_USERNAME_LENGTH];
        DWORD UIdFrom;        // UniqID of poster

        time_t Date;        // post date
        DWORD TTL;                // Time To Live of this announce

        DWORD Flags;        // flags of the post

        DWORD Number;        // unique announce number (for identification)

        DWORD Reserved[2];        // reserved for the future use
};
#pragma pack(4)

/* Post announce as selected user.
 * It don't check security permitions of poster !
 */
int PostGlobalAnnounce(char *username, DWORD uniqid, char *announce, DWORD ttl, DWORD flags);

/* Read annonces that implies to 'ct' date. (!!! ignored for now !!!)
 */
int ReadGlobalAnnounces(time_t ct, SGlobalAnnounce **ga, DWORD *cnt);

/* Delete global announce by id
 */
int DeleteGlobalAnnounce(DWORD id, DWORD uniqid);

/* Read last used announce number
 */
int ReadLastAnnounceNumber(DWORD *an);

/* Update announce (using updateopt flags) by id
 */
int UpdateGlobalAnnounce(DWORD id, char *username, DWORD uniqid, char *announce,
                DWORD ttl, DWORD flags, DWORD updateopt);

#endif // of ANNOUNCES_H_INCLUDED
