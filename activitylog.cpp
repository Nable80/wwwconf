/***************************************************************************
                          activitylog.cpp  -  board activity logger
                             -------------------
    begin                : Mon Oct 6 2003
    copyright            : (C) 2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "activitylog.h"
#include "error.h"
#include <algorithm>

#define ACTIVITY_CONTROL_TIME        10*60        // 10min
#define BLOCKREADCOUNT        1000

int RegisterActivityFrom(DWORD IP, DWORD &hostcnt, DWORD &hitcnt)
{
        WCFILE *f = NULL, *f1 = NULL;
        DWORD tm, tm1, fin, fin1;
        int disfileok = 0, addfileok = 0, finalizedone = 0, swapdone = 0;
        hostcnt = hitcnt = 0;
        DWORD crtime = (DWORD)time(NULL);

        if((f = wcfopen(F_ACTIVITYLOG1, FILE_ACCESS_MODES_RW)) == NULL) {
                // create file
                if ((f = wcfopen(F_ACTIVITYLOG1, FILE_ACCESS_MODES_CW)) == NULL) {
                        goto failed;
                }
                lock_file(f);
                DWORD x[2] = {0, 0};
                if (!fCheckedWrite(x, sizeof(x), f) || wcfseek(f, 0, SEEK_SET)) {
                        goto failed;
                }
        }
        else lock_file(f);

        if((f1 = wcfopen(F_ACTIVITYLOG2, FILE_ACCESS_MODES_RW)) == NULL) {
                // create file
                if ((f1 = wcfopen(F_ACTIVITYLOG2, FILE_ACCESS_MODES_CW)) == NULL) {
                        goto failed;
                }
                lock_file(f1);
                DWORD x[2] = {0, 0};
                if (!fCheckedWrite(x, sizeof(x), f1) || wcfseek(f1, 0, SEEK_SET)) {
                        goto failed;
                }
        }
        else lock_file(f1);

        if(!fCheckedRead(&tm, 4, f))
                goto failed;
        if(!fCheckedRead(&tm1, 4, f1))
                goto failed;
        if(!fCheckedRead(&fin, 4, f))
                goto failed;
        if(!fCheckedRead(&fin1, 4, f1))
                goto failed;

        // find file for displaying (it will be f1)
        if(crtime - 2*ACTIVITY_CONTROL_TIME <= tm  && tm < crtime - ACTIVITY_CONTROL_TIME )
        {
                std::swap(f, f1);
                std::swap(tm, tm1);
                std::swap(fin, fin1);

                // to know path to the file
                swapdone = !swapdone;

                disfileok = 1;
        }
        if(crtime - 2*ACTIVITY_CONTROL_TIME <= tm1 && tm1 < crtime - ACTIVITY_CONTROL_TIME )
        {
                disfileok = 1;
        }

        // find file for saving (will be f)
        if(crtime - ACTIVITY_CONTROL_TIME <= tm && tm < crtime)
        {
                addfileok = 1;
        }
        if(crtime - ACTIVITY_CONTROL_TIME <= tm1 && tm1 < crtime)
        {
                std::swap(f, f1);
                std::swap(tm, tm1);
                std::swap(fin, fin1);

                // to know path to the file
                swapdone = !swapdone;

                addfileok = 1;
        }

        // parse file with display information
        if(disfileok) {
                if(!fin1) {
                        DWORD rr, cn, i;
                        SActivityLogRecord* buf;

                        // finalize file
                        buf = (SActivityLogRecord*)malloc(sizeof(SActivityLogRecord)*BLOCKREADCOUNT);
                        while(!wcfeof(f1)) {
                                rr = wcfread(buf, 1, sizeof(SActivityLogRecord)*BLOCKREADCOUNT, f1);

                                if((rr%sizeof(SActivityLogRecord)) != 0) {
                                        /* read error */
                                        break;
                                }
                                cn = rr/sizeof(SActivityLogRecord);
                                hostcnt += cn;
                                for(i = 0; i < cn; i++) {
                                        hitcnt += buf[i].Count;
                                }
                        }
                        free(buf);
                        // truncate end of file
                        if (wcfflush(f1) || ftruncate(fileno(f1), sizeof(DWORD)) || wcfseek(f1, 0, SEEK_END)) {
                                printhtmlerror();
                        }
                        rr = 1;
                        if (!fCheckedWrite(&rr, sizeof(rr), f1)
                                || !fCheckedWrite(&hitcnt, sizeof(hitcnt), f1)
                                || !fCheckedWrite(&hostcnt, sizeof(hostcnt), f1)) {
                                goto failed;
                        }
                        finalizedone = 1;
                }
                else {
                        // just read info from file
                        if (!fCheckedRead(&hitcnt, sizeof(hitcnt), f1) || !fCheckedRead(&hostcnt, sizeof(hostcnt), f1)) {
                                goto failed;
                        }
                }
                // release file lock as soon as possible (won't wait for the end of this fuction)
                unlock_file(f1);
                wcfclose(f1);
                f1 = NULL;
        }

        if(addfileok) {
                DWORD rr, cn, pos, done = 0, i;
                SActivityLogRecord* buf;

                // add ip to file
                buf = (SActivityLogRecord*)malloc(sizeof(SActivityLogRecord)*BLOCKREADCOUNT);
                while(!wcfeof(f)) {
                        pos = wcftell(f);
                        rr = wcfread(buf, 1, sizeof(SActivityLogRecord)*BLOCKREADCOUNT, f);

                        if((rr%sizeof(SActivityLogRecord)) != 0) {
                                /* read error */
                                done = 1;
                                break;
                        }
                        cn = rr/sizeof(SActivityLogRecord);
                        for(i = 0; i < cn; i++) {
                                if(buf[i].IP == IP) {
                                        buf[i].Count++;
                                        buf[i].Time = (DWORD)time(NULL);
                                        done = 1;
                                        if (wcfseek(f, pos, SEEK_SET) || !fCheckedWrite(buf, sizeof(SActivityLogRecord)*(i+1), f)) {
                                                goto failed;
                                        }
                                        break;
                                }
                        }
                        if(done) break;
                }
                free(buf);
                if(!done) {
                        SActivityLogRecord ss;
                        ss.Count = 1;
                        ss.IP = IP;
                        ss.Time = crtime;
                        if (wcfseek(f, 0, SEEK_END) || !fCheckedWrite(&ss, sizeof(SActivityLogRecord), f)) {
                                goto failed;
                        }
                }
        }
        else {
                // if it's filalized file, let's add activity to the history
                if(fin) {
                        WCFILE *f2;
                        // add to achive
                        if (!fCheckedRead(&hitcnt, sizeof(hitcnt), f) || !fCheckedRead(&hostcnt, sizeof(hostcnt), f)) {
                                goto failed;
                        }

                        if((f2 = wcfopen(F_ACTIVITYARCH, FILE_ACCESS_MODES_RW)) == NULL) {
                                // create file
                                f2 = wcfopen(F_ACTIVITYARCH, FILE_ACCESS_MODES_CW);
                        }

                        if(f2 != NULL) {
                                lock_file(f2);
                                if (wcfseek(f2, 0, SEEK_END) == 0) {
                                        off_t saved_end = wcftell(f2);
                                        DWORD buf[3] = {tm, hitcnt, hostcnt};
                                        if (!fCheckedWrite(buf, sizeof(buf), f2) || wcfflush(f2)) {
                                                if (ftruncate(fileno(f2), saved_end)) {
                                                        printhtmlerror();
                                                }
                                        }
                                }
                                unlock_file(f2);
                                wcfclose(f2);
                        }
                }

                if (wcfflush(f) || ftruncate(fileno(f), sizeof(DWORD)) || wcfseek(f, 0, SEEK_END)) {
                        printhtmlerror();
                }

                DWORD rr = 0;
                SActivityLogRecord ss;
                ss.Count = 1;
                ss.IP = IP;
                ss.Time = crtime;
                if (!fCheckedWrite(&rr, sizeof(rr), f) || !fCheckedWrite(&ss, sizeof(ss), f)) {
                        goto failed;
                }
                // make header
                if (wcfseek(f, 0, SEEK_SET) == 0) {
                        rr = finalizedone ? (tm1 + ACTIVITY_CONTROL_TIME) : crtime;
                        if (!fCheckedWrite(&rr, sizeof(rr), f)) {
                                goto failed;
                        }
                }
        }
failed:
        if(f) {
                unlock_file(f);
                wcfclose(f);
        }
        if(f1) {
                unlock_file(f1);
                wcfclose(f1);
        }
        return 0;
}
