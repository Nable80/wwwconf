#include <sys/statvfs.h>
#include "statfs.h"

bool isEnoughSpace(void)
{
        struct statvfs fsi;
        if (statvfs(".", &fsi))
                return false;
        return fsi.f_bavail > 1 + MINSIZE / fsi.f_bsize;
}
