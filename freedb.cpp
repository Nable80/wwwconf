/***************************************************************************
                          freedb.cpp  -  free spaces database support
                             -------------------
    begin                : Sun Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "freedb.h"

#define unlock_and_freedb_io_error1() {unlock_file(ffb);errnum = FREEDBFILE_ERROR_IO_ERROR;return FREEDBFILE_ERROR_IO_ERROR;}
#define freedb_io_error1() {errnum = FREEDBFILE_ERROR_IO_ERROR; return FREEDBFILE_ERROR_IO_ERROR;}
#define freedb_lock_file() {lock_file(ffb);}
#define freedb_unlock_file() {unlock_file(ffb)}


/* mark free space block with size bsize and index bIndex
 * if successfull return FREEDB_ERROR_ALLOK
 */
DWORD CFreeDBFile::MarkFreeSpace(DWORD bIndex, DWORD bsize)
{
	SFreeDBEntry fs;
	DWORD rr;
	WCFILE *ffb;

	if(!init) {
		errnum = FREEDBFILE_ERROR_NOT_INITIALIZED;
		return errnum;
	}

	// ignore wasted blocks
	if(bsize < wasted_block) bsize = 0;

	if((ffb = wcfopen(fname, FILE_ACCESS_MODES_RW)) == NULL) {
		if((ffb = wcfopen(fname, FILE_ACCESS_MODES_CW)) == NULL) {
			freedb_io_error1()
		}
		else {
			wcfclose(ffb);
			if((ffb = wcfopen(fname, FILE_ACCESS_MODES_RW)) == NULL) {
				freedb_io_error1()
			}
		}
	}
	/* seek at begining of WCFILE */
	if(wcfseek(ffb, 0, SEEK_SET) != 0) {
		freedb_io_error1();
	}

	/********* lock WCFILE *********/
	freedb_lock_file();

	int alreadyfind = 0;
	while(!wcfeof(ffb)) {
		if((rr = (DWORD)wcfread(&fs, 1, sizeof(SFreeDBEntry), ffb)) != sizeof(SFreeDBEntry)) {
			if(rr == 0) break;
			unlock_and_freedb_io_error1();
		}
		if(fs.size == 0) {
			alreadyfind = 1;
			rr = wcftell(ffb) - sizeof(SFreeDBEntry);
			if(wcfseek(ffb, rr, SEEK_SET) != 0) {
				unlock_and_freedb_io_error1();
			}
			fs.size = bsize;
			fs.index = bIndex;
			if(!fCheckedWrite(&fs, sizeof(SFreeDBEntry), ffb)) {
				unlock_and_freedb_io_error1();
			}
			break;
		}
	}
	if(!alreadyfind) {
		if(wcfseek(ffb, 0, SEEK_END) != 0) {
			unlock_and_freedb_io_error1()
		}
		fs.size = bsize;
		fs.index = bIndex;
		if(!fCheckedWrite(&fs, sizeof(SFreeDBEntry), ffb)) {
			unlock_and_freedb_io_error1()
		}
	}
	freedb_unlock_file();
	/******* unlock ffb WCFILE *******/
	wcfclose(ffb);
	errnum = FREEDBFILE_ERROR_ALLOK;
	return FREEDBFILE_ERROR_ALLOK;
}

/* allocate size bytes of free space, and return index of it
 * otherwise return 0xFFFFFFFF and set errnum with error code
 */
DWORD CFreeDBFile::AllocFreeSpace(DWORD size)
{
	SFreeDBEntry fs;
	DWORD rr;
	WCFILE *ffb;
	
	if(!init) {
		errnum = FREEDBFILE_ERROR_NOT_INITIALIZED;
		return 0xFFFFFFFF;
	}

	if((ffb = wcfopen(fname, FILE_ACCESS_MODES_RW)) == NULL) {
		if((ffb = wcfopen(fname, FILE_ACCESS_MODES_CW)) == NULL) {
			freedb_io_error1()
		}
		else {
			wcfclose(ffb);
			if((ffb = wcfopen(fname, FILE_ACCESS_MODES_RW)) == NULL) {
				freedb_io_error1()
			}
		}
	}
	/* seek at begining of WCFILE */
	if(wcfseek(ffb, 0, SEEK_SET) != 0) {
		freedb_io_error1();
	}

	/******** lock ffb *********/
	freedb_lock_file();

	while(!wcfeof(ffb)) {
		if((rr = (DWORD)wcfread(&fs, 1, sizeof(SFreeDBEntry), ffb)) != sizeof(SFreeDBEntry)) {
			if(rr == 0) {
				break;
			}
			unlock_and_freedb_io_error1();
		}
		if(fs.size >= size) {
			rr = wcftell(ffb) - sizeof(SFreeDBEntry);
			if(wcfseek(ffb, rr, SEEK_SET) != 0) {
				unlock_and_freedb_io_error1()
			}

			fs.size = fs.size - size;
			fs.index = fs.index + size;

			// make wasted space, if block too small
			if(fs.size < wasted_block) fs.size = 0;
			
			if(!fCheckedWrite(&fs, sizeof(SFreeDBEntry), ffb)) {
				unlock_and_freedb_io_error1()
			}
			
			freedb_unlock_file();
			wcfclose(ffb);
			// unlock FFB semaphore
			return fs.index - size;
		}
	}

	freedb_unlock_file();
	/********* unlock ffb *********/

	wcfclose(ffb);

	return 0xFFFFFFFF;
}

CFreeDBFile::CFreeDBFile(char *ifname, DWORD wasted_b)
{
	init = 0;
	if(ifname == NULL || strlen(ifname) < 1) {
		errnum = FREEDBFILE_ERROR_INVALID_FILE;
		return;
	}
	fname = (char*)malloc(strlen(ifname) + 1);
	strcpy(fname, ifname);
	wasted_block = wasted_b;
	errnum = FREEDBFILE_ERROR_ALLOK;
	init = 1;
}

CFreeDBFile::~CFreeDBFile()
{
	if(init) {
		free(fname);
	}
}
