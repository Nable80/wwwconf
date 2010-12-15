/***************************************************************************
                          freedb.h  -  free spaces database support include
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef FREEDBFILE_H_INCLUDED
#define FREEDBFILE_H_INCLUDED

#include "basetypes.h"

#define FREEDBFILE_ERROR_ALLOK					0
#define FREEDBFILE_ERROR_NOT_INITIALIZED		1
#define FREEDBFILE_ERROR_IO_ERROR				2
#define FREEDBFILE_ERROR_INVALID_FILE			3

#define FREEDBFILE_NO_FREE_SPACE		0xFFFFFFFF

#define FREEDBFILE_READBUF_COUNT				20

/* free spaces index table entry */
struct SFreeDBEntry {
	DWORD size;
	DWORD index;
};

class CFreeDBFile {
protected:
	char *fname;
	int init;
	DWORD wasted_block;
public:
	CFreeDBFile(const char *ifname, DWORD wasted_block);
	~CFreeDBFile();
	DWORD errnum;
	DWORD MarkFreeSpace(DWORD bIndex, DWORD bsize);
	DWORD AllocFreeSpace(DWORD size);
};

#endif
