/***************************************************************************
                     colornick.cpp  -  alternative nick spelling support
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2001-2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "colornick.h"

CAltNamesParser::CAltNamesParser(const char *fname, bool &init)
{
	init = 0;
	classinit = 0;
	AltNamesStruct ns;
	char *ts;
	DWORD rd;

	// try to open existing file
	if((f = wcfopen(fname, FILE_ACCESS_MODES_RW)) == NULL) {
		// not exist - try to create file
		if((f = wcfopen(fname, FILE_ACCESS_MODES_CW)) == NULL) {
			return;
		}
		else {
			wcfclose(f);
			if((f = wcfopen(fname, FILE_ACCESS_MODES_RW)) == NULL) {
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
		ts = (char*)malloc(MAX_ALT_NICK_SIZE);
		strcpy(ts, ns.aname);
		nmap[ns.uid] = ts;
	}
	wcfclose(f);

	strcpy(ifname, fname);

	init = 1;
	classinit = 1;
}

CAltNamesParser::~CAltNamesParser()
{
	std::unordered_map<DWORD, char*>::iterator it;
	for(it = nmap.begin(); it != nmap.end(); it++) {
		free(it->second);
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
			strcpy(s2, altname);
			nmap[uid] = s2;
			// save to file
			ns.uid = uid;
			strcpy(ns.rname, name);
			strcpy(ns.aname, s2);
			if((f = wcfopen(ifname, FILE_ACCESS_MODES_RW)) == NULL)
				return 0; 	// file MUST exist
			lock_file(f);
			wcfseek(f, 0, SEEK_END);
			fCheckedWrite(&ns, sizeof(ns), f);
			unlock_file(f);
			wcfclose(f);
			return 1;
		}
		else {
			// update
			AltNamesStruct ns;
			memset(&ns, 0, sizeof(ns));
			DWORD pos, fn = 0;
			char *s1, *s2 = (char*)malloc(MAX_ALT_NICK_SIZE);
			strcpy(s2, altname);
			s1 = nmap[uid];
			free(s1);
			nmap[uid] = s2;
			// save to file (first of all - let's find)
			if((f = wcfopen(ifname, FILE_ACCESS_MODES_RW)) == NULL)
				return 0; 	// file MUST exist
			lock_file(f);
			while(!wcfeof(f)) {
				pos = wcftell(f);
				if(!fCheckedRead(&ns, sizeof(ns), f)) {
					wcfclose(f);
					return 0;
				}
				if(ns.uid == uid) {
					strcpy(ns.rname, name);
					strcpy(ns.aname, altname);
					wcfseek(f, pos, SEEK_SET);
					fn = 1;
					break;
				}
			}
			if(fn) fCheckedWrite(&ns, sizeof(ns), f);
			unlock_file(f);
			wcfclose(f);
			return 1;
		}
	}
	return 0;	// Already exist
}

int CAltNamesParser::DeleteAltName(DWORD uid)
{
	if(classinit) {
		std::unordered_map<DWORD, char*>::iterator it;
		if(nmap.find(uid) != nmap.end()) {
			AltNamesStruct ns;
			DWORD pos, rd, fn = 0;
			char cb[100000];
			// delete from file (first of all - let's find)
			if((f = wcfopen(ifname, FILE_ACCESS_MODES_RW)) == NULL)
				return 0; 	// file MUST exist
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
			if(fn) fCheckedWrite(cb, rd, f);
			pos = wcftell(f);
			truncate(ifname, pos);
			unlock_file(f);
			wcfclose(f);
		
			it = nmap.find(uid);
			free(it->second);
			nmap.erase(it);

			return 1;
		}
	}
	return 0;
}

int CAltNamesParser::NameToAltName(DWORD uid, char *altname)
{
	if(classinit) {
		if(nmap.find(uid) != nmap.end()) {
			strcpy(altname, nmap[uid]);
			return 1;
		}
	}
	return 0;
}
