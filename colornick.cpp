/***************************************************************************
                     colornick.cpp  -  alternative nick spelling support
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2001-2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "colornick.h"
#include "error.h"

#if USER_ALT_NICK_SPELLING_SUPPORT
CAltNamesParser AltNames;
#endif

CAltNamesParser::CAltNamesParser()
{
        classinit = 0;
        AltNamesStruct ns;
        DWORD rd;

        // try to open existing file
        if((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL) {
                // not exist - try to create file
                if((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_CW)) == NULL) {
                        return;
                }
                else {
                        wcfclose(f);
                        if((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL) {
                                return;
                        }
                }
        }

        // file successfully opened - read values
        while(!wcfeof(f)) {
                if(((rd = (DWORD)wcfread(&ns, 1, sizeof(ns), f)) % sizeof(ns)) != 0) {
                        wcfclose(f);
                        return;
                }
                if(!rd) break;
                char *ts = strdup(ns.aname);
                // there shouldn't be any duplicates here but let's check it anyway
                if (!nmap.emplace(ns.uid, ts).second) {
                        print2log("duplicate altname record for uid %lu: '%s'", ns.uid, ts);
                        free(ts);
                }
        }
        wcfclose(f);

        classinit = 1;
}

CAltNamesParser::~CAltNamesParser()
{
        for (auto& it: nmap) {
                free(it.second);
                // GCC's static analyzer complains about possible double-free
                // without this line. Who can explain it?
                it.second = NULL;
        }
        nmap.clear();
}

int CAltNamesParser::AddAltName(DWORD uid, char *name, char *altname)
{
        if(classinit) {
                if(nmap.find(uid) == nmap.end()) {
                        AltNamesStruct ns;
                        memset(&ns, 0, sizeof(ns));
                        char *s2 = (char*)malloc(MAX_ALT_NICK_SIZE);
                        if (s2 == NULL) {
                                return 0;
                        }
                        strcpy(s2, altname);
                        nmap[uid] = s2;
                        // save to file
                        ns.uid = uid;
                        strcpy(ns.rname, name);
                        strcpy(ns.aname, s2);
                        if((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL)
                                return 0;         // file MUST exist
                        lock_file(f);
                        int res = 0;
                        if (wcfseek(f, 0, SEEK_END) == 0 && fCheckedWrite(&ns, sizeof(ns), f)) {
                                res = 1;
                        }
                        unlock_file(f);
                        wcfclose(f);
                        return res;
                }
                else {
                        // update
                        AltNamesStruct ns;
                        memset(&ns, 0, sizeof(ns));
                        char *s1, *s2 = (char*)malloc(MAX_ALT_NICK_SIZE);
                        if (s2 == NULL) {
                                return 0;
                        }
                        strcpy(s2, altname);
                        s1 = nmap[uid];
                        free(s1);
                        nmap[uid] = s2;
                        // save to file (first of all - let's find)
                        if((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL)
                                return 0;         // file MUST exist
                        lock_file(f);
                        int res = 0;
                        while(!wcfeof(f)) {
                                DWORD pos = wcftell(f);
                                if(!fCheckedRead(&ns, sizeof(ns), f)) {
                                        wcfclose(f);
                                        return 0;
                                }
                                if(ns.uid == uid) {
                                        strcpy(ns.rname, name);
                                        strcpy(ns.aname, altname);
                                        if (wcfseek(f, pos, SEEK_SET) == 0 && fCheckedWrite(&ns, sizeof(ns), f)) {
                                                res = 1;
                                        }
                                        break;
                                }
                        }
                        unlock_file(f);
                        wcfclose(f);
                        return res;
                }
        }
        return 0;        // Already exist
}

int CAltNamesParser::DeleteAltName(DWORD uid)
{
        if(classinit) {
                auto it = nmap.find(uid);
                if(it != nmap.end()) {
                        AltNamesStruct ns;
                        DWORD pos, rd, fn = 0;
                        int res = 1;
                        // FIXME
                        char cb[100000];
                        // delete from file (first of all - let's find)
                        if((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL)
                                return 0;         // file MUST exist
                        lock_file(f);
                        while(!wcfeof(f)) {
                                pos = wcftell(f);
                                if(!fCheckedRead(&ns, sizeof(ns), f)) {
                                        wcfclose(f);
                                        return 0;
                                }
                                if(ns.uid == uid) {
                                        rd = (DWORD)wcfread(cb, 1, 100000, f);
                                        if(rd == 100000) {
                                                wcfclose(f);
                                                return 0;
                                        }
                                        wcfseek(f, pos, SEEK_SET);
                                        fn = 1;
                                        break;
                                }
                        }
                        if (fn) {
                                if (fCheckedWrite(cb, rd, f)) {
                                        pos = wcftell(f);
                                        if (wcfflush(f) || ftruncate(fileno(f), pos)) {
                                                printhtmlerror();
                                        }
                                }
                                else {
                                        res = 0;
                                }
                        }
                        unlock_file(f);
                        wcfclose(f);

                        free(it->second);
                        nmap.erase(it);

                        return res;
                }
        }
        return 0;
}

int CAltNamesParser::NameToAltName(DWORD uid, char *altname)
{
        if(classinit) {
                const auto it = nmap.find(uid);
                if(it != nmap.end() && it->second) {
                        strcpy(altname, it->second);
                        return 1;
                }
        }
        return 0;
}
