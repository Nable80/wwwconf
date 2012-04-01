#include <stdio.h>
#include "basetypes.h"
#include "error.h"
#include "messages.h"

DWORD Fsize(const char *s)
{
        WCFILE *f;
        register DWORD r;
        if((f = wcfopen(s, FILE_ACCESS_MODES_R)) == NULL) {
                        char ss[10000];
                        sprintf(ss, LOG_GETFILESIZEFAILED, s);
                        printhtmlerrormes(ss);
        }
        if(wcfseek(f, 0, SEEK_END) != 0) {
                        char ss[10000];
                        sprintf(ss, LOG_GETFILESIZEFAILED, s);
                        printhtmlerrormes(ss);
        }
        r = wcftell(f);
        wcfclose(f);
        return r;
}

int ReadDBMessage(DWORD midx, SMessage *mes)
{
        WCFILE *f;

        if(midx == NO_MESSAGE_CODE) return 0;
        if((f = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_R)) == NULL) return 0;
        if(wcfseek(f, midx, SEEK_SET) != 0)
        {
                wcfclose(f);
                return 0;
        }
        if(!fCheckedRead(mes, sizeof(SMessage), f))
        {
                wcfclose(f);
                return 0;
        }
        wcfclose(f);
        return 1;
}

int ReadDBMessageBody(char *buf, DWORD index, DWORD size)
{
        FILE *f;
        if((f = fopen(F_MSGBODY, FILE_ACCESS_MODES_R)) == NULL)return 0;
        if(fseek(f, index, SEEK_SET) != 0) return 0;
        if(fread(buf, 1, size, f) != size) return 0;
        fclose(f);
        return 1;
}

int WriteDBMessage(DWORD midx, SMessage *mes)
{
        WCFILE *f;
        
        if((f = wcfopen(F_MSGINDEX, FILE_ACCESS_MODES_RW)) == NULL) return 0;
        lock_file(f);
        if(wcfseek(f, midx, SEEK_SET) != 0)
        {
                unlock_file(f);
                wcfclose(f);
                return 0;
        }
        if(!fCheckedWrite(mes, sizeof(SMessage), f))
        {
                unlock_file(f);
                wcfclose(f);
                return 0;
        }
        unlock_file(f);
        wcfclose(f);
        return 1;
}

int WriteDBMessageBody(char *buf, DWORD index, DWORD size)
{
        FILE *f;
        if((f = fopen(F_MSGBODY, FILE_ACCESS_MODES_RW)) == NULL)return 0;
        if(fseek(f, index, SEEK_SET) != 0) return 0;
        if(fwrite(buf, 1, size, f) != size) return 0;
        fclose(f);
        return 1;
}

DWORD VIndexCountInDB()
{        
        DWORD fsize;
        fsize = Fsize(F_VINDEX);
        if (fsize > 0)
                return ( (DWORD)(((DWORD)fsize) - 3) )/sizeof(DWORD);
        else 
                return 0;

}

DWORD TranslateMsgIndexDel(DWORD root)
{
        DWORD i;
        size_t n;
        FILE *f;

        if (root == 0)
                return NO_MESSAGE_CODE;
        
        if ( (f = wcfopen(F_VINDEX, FILE_ACCESS_MODES_R)) == NULL)
                printhtmlerror();

        if (wcfseek(f, root*sizeof(DWORD), SEEK_SET))
                printhtmlerror();

        n = fread(&i, 1, sizeof(i), f);

        if (ferror(f))
                printhtmlerror();

        if (n != sizeof(i)) {
                if (n == 0 && feof(f))
                        i = NO_MESSAGE_CODE;
                else
                        printhtmlerror();
        } else if (i == NO_MESSAGE_CODE)
                i = 0;
        
        wcfclose(f);

        return i;
}
