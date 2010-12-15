#include <stdio.h>
#include <sys/statfs.h>
#include "statfs.h" 

typedef signed long long i64;

bool isEnoughSpace(void)
{
	const i64 minsize = 10 * 1000 * 1000; //10Mb
	struct statfs fsi;
	if(statfs(".", &fsi))
		return false;
//	printf(
//		"fs stats: fsi.f_bsize: %u; fsi.f_bavail: %u\n", 
//		fsi.f_bsize, 
//		fsi.f_bavail);
	return (i64)fsi.f_bsize * (i64)fsi.f_bavail > minsize;
}
