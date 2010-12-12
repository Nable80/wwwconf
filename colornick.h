/***************************************************************************
               colornick.h  -  alternative nick spelling support include
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2001-2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef COLORNICK_H_INCLUDED
#define COLORNICK_H_INCLUDED

#include "basetypes.h"

#if defined(__GNUC__)
#  if __GNUC__ < 3 && __GNUC__ >= 2 && __GNUC_MINOR__ >= 95
#     include <hash_map>
#  elif __GNUC__ >= 3
#     include <ext/hash_map>
namespace std{
	using namespace __gnu_cxx;
}
#  else
#     include <hash_map.h>
#  endif
#elif defined(_MSC_VER)
#  if _MSC_VER >= 1100
#     include <hash_map>
#  else
#     error "std::hash_map is not available with this compiler, compile using MSVC7 and later"
#  endif
#elif defined(__sgi__)
#  include <hash_map>
#else
#  error "std::hash_map is not available with this compiler"
#endif

#define MAX_REAL_NICK_SIZE	30
#define MAX_ALT_NICK_SIZE	300

class CAltNamesParser {
private:
	int classinit;
	WCFILE *f;
	char ifname[1000];
	std::hash_map<DWORD, char*> nmap;
#pragma pack(1)
	typedef struct _AltNamesStruct {
		DWORD uid;
		char rname[MAX_REAL_NICK_SIZE];
		char aname[MAX_ALT_NICK_SIZE];
	} AltNamesStruct, *PAltNamesStruct;
#pragma pack(4)
public:
	CAltNamesParser(char *fname, bool &init);
	~CAltNamesParser();

	int AddAltName(DWORD uid, char *name, char *altname);
	int DeleteAltName(DWORD uid);
	int NameToAltName(DWORD uid, char *altname);
};

#endif
