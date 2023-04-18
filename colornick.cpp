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
        WCFILE *f;

        // Try to open existing file:
        if ((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_R)) == NULL) {
                throw std::system_error(errno, std::generic_category(), F_PROF_ALTNICK);
        }

        // Read entries into in-memory cache:
        lock_file(f);
        while (1) {
                AltNamesStruct ns;
                if (!fCheckedRead(&ns, sizeof(ns), f)) {
                        break;
                }
                char *ts = strdup(ns.aname);
                // Try to insert while checking for possible duplicates:
                if (!nmap.emplace(ns.uid, ts).second) {
                        print2log("duplicate altname record for uid %lu: '%s'", ns.uid, ts);
                        free(ts);
                }
        }
        // Unlock and close:
        wcfclose(f);
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

int CAltNamesParser::AddAltName(DWORD uid, const char *name, const char *altname)
{
        AltNamesStruct ns = {};
        WCFILE *f;
        int res = 0;

        // Update in-memory cache:
        auto it = nmap.find(uid);
        if (it != nmap.end()) {
                free(it->second);
        }
        nmap[uid] = strdup(altname);

        // Open file for update:
        if ((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL) {
                return 0;
        }
        lock_file(f);

        // Find the corresponding record for update, else append to the end of file:
        off_t pos;
        while (1) {
                pos = wcftell(f);
                if (!fCheckedRead(&ns, sizeof(ns), f)) {
                        // Exit on read error:
                        if (ferror(f)) {
                                wcfclose(f);
                                return 0;
                        }
                        // EOF, append record:
                        break;
                }
                if (ns.uid == uid) {
                        // Found it, let's update:
                        break;
                }
        }

        // Write record:
        ns.uid = uid;
        strcpy(ns.rname, name);
        strcpy(ns.aname, altname);
        if (wcfseek(f, pos, SEEK_SET) == 0 && fCheckedWrite(&ns, sizeof(ns), f)) {
                res = 1;
        }

        // Flush, unlock, close:
        unlock_file(f);
        wcfclose(f);
        return res;
}

int CAltNamesParser::DeleteAltName(DWORD uid)
{
        auto it = nmap.find(uid);
        if (it == nmap.end()) {
                return 0;
        }
        // Remove from in-memory cache:
        free(it->second);
        nmap.erase(it);

        WCFILE *f;
        AltNamesStruct ns;
        DWORD pos, rd, fn = 0;
        int res = 1;
        // FIXME
        char cb[100000];
        // Open file for update:
        if ((f = wcfopen(F_PROF_ALTNICK, FILE_ACCESS_MODES_RW)) == NULL) {
                return 0;
        }

        lock_file(f);
        while (!wcfeof(f)) {
                pos = wcftell(f);
                if (!fCheckedRead(&ns, sizeof(ns), f)) {
                        wcfclose(f);
                        return 0;
                }
                if (ns.uid == uid) {
                        rd = (DWORD)wcfread(cb, 1, 100000, f);
                        if (rd == 100000) {
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

        return res;
}

int CAltNamesParser::NameToAltName(DWORD uid, char *altname)
{
        const auto it = nmap.find(uid);
        if (it != nmap.end() && it->second) {
                strcpy(altname, it->second);
                return 1;
        }
        return 0;
}
