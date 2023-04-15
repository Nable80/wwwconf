/***************************************************************************
               colornick.h  -  alternative nick spelling support include
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2001-2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef COLORNICK_H_INCLUDED
#define COLORNICK_H_INCLUDED

#include <unordered_map>
#include "basetypes.h"

#define MAX_REAL_NICK_SIZE        30
#define MAX_ALT_NICK_SIZE        300

class CAltNamesParser {
private:
        int classinit;
        WCFILE *f;
        std::unordered_map<DWORD, char*> nmap;
#pragma pack(push, 1)
        typedef struct _AltNamesStruct {
                DWORD uid;
                char rname[MAX_REAL_NICK_SIZE];
                char aname[MAX_ALT_NICK_SIZE];
        } AltNamesStruct, *PAltNamesStruct;
#pragma pack(pop)
public:
        CAltNamesParser();
        ~CAltNamesParser();

        int AddAltName(DWORD uid, char *name, char *altname);
        int DeleteAltName(DWORD uid);
        int NameToAltName(DWORD uid, char *altname);
};

#if USER_ALT_NICK_SPELLING_SUPPORT
extern CAltNamesParser AltNames;
#endif

#endif
