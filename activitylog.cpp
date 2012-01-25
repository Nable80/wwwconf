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
        DWORD crtime = time(NULL);

        if((f = wcfopen(F_ACTIVITYLOG1, FILE_ACCESS_MODES_RW)) == NULL) {
                // create file
                if((f = wcfopen(F_ACTIVITYLOG1, FILE_ACCESS_MODES_CW)) != NULL)
                {
                        DWORD x[2];
                        memset(x,0,8);
                        fCheckedWrite(&x, 8, f);
                        wcfclose(f);
                        f = wcfopen(F_ACTIVITYLOG1, FILE_ACCESS_MODES_RW);
                        if(f) lock_file(f);
                }
                else goto failed;
        }
        else lock_file(f);
        if((f1 = wcfopen(F_ACTIVITYLOG2, FILE_ACCESS_MODES_RW)) == NULL) {
                // create file
                if((f1 = wcfopen(F_ACTIVITYLOG2, FILE_ACCESS_MODES_CW)) != NULL)
                {
                        DWORD x[2];
                        memset(x,0,8);
                        fCheckedWrite(&x, 8, f1);
                        wcfclose(f1);
                        f1 = wcfopen(F_ACTIVITYLOG2, FILE_ACCESS_MODES_RW);
                        if(f1) lock_file(f1);
                }
                else goto failed;
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
                        wcfflush(f1);
                        // truncate end of file
                        if (truncate(swapdone ? F_ACTIVITYLOG1 : F_ACTIVITYLOG2, 4)) {
                                unlock_file(f);
                                unlock_file(f1);
                                printhtmlerror();
                        }
                        wcfseek(f1, 0, SEEK_END);
                        rr = 1;
                        fCheckedWrite(&rr, 4, f1);
                        fCheckedWrite(&hitcnt, 4, f1);
                        fCheckedWrite(&hostcnt, 4, f1);
                        finalizedone = 1;
                }
                else {
                        // just read info from file
                        fCheckedRead(&hitcnt, 4, f1);
                        fCheckedRead(&hostcnt, 4, f1);
                }
                // release file lock as soon as possible (won't wait for the end of this fuction)
                wcfflush(f1);
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
                                        buf[i].Time = time(NULL);
                                        done = 1;
                                        wcfseek(f, pos, SEEK_SET);
                                        fCheckedWrite(buf, sizeof(SActivityLogRecord)*(i+1), f);
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
                        wcfseek(f, 0, SEEK_END);
                        fCheckedWrite(&ss, sizeof(SActivityLogRecord), f);
                }
        }
        else {
                DWORD rr;
                SActivityLogRecord ss;

                // if it's filalized file, let's add activity to the history
                if(fin) {
                        WCFILE *f2;
                        DWORD buf[3];
                        // add to achive 
                        fCheckedRead(&hitcnt, 4, f);
                        fCheckedRead(&hostcnt, 4, f);

                        if((f2 = wcfopen(F_ACTIVITYARCH, FILE_ACCESS_MODES_RW)) == NULL) {
                                // create file
                                f2 = wcfopen(F_ACTIVITYARCH, FILE_ACCESS_MODES_CW);
                        }

                        if(f2 != NULL) {
                                lock_file(f2);
                                wcfseek(f2, 0, SEEK_END);
                                buf[0] = tm; buf[1] = hitcnt; buf[2] = hostcnt;
                                fCheckedWrite(&buf, 12, f2);
                                wcfflush(f2);
                                unlock_file(f2);
                                wcfclose(f2);
                        }
                }

                ss.Count = 1;
                ss.IP = IP;
                ss.Time = crtime;

                wcfflush(f);
                if (truncate(swapdone ? F_ACTIVITYLOG2 : F_ACTIVITYLOG1, 4)) {
                        unlock_file(f);
                        unlock_file(f1);
                        printhtmlerror();
                }
                wcfseek(f, 4, SEEK_SET);
                rr = 0;
                fCheckedWrite(&rr, 4, f);
                fCheckedWrite(&ss, sizeof(SActivityLogRecord), f);
                // make header
                wcfseek(f, 0, SEEK_SET);
                if(!finalizedone) rr = crtime;
                else rr = tm1 + ACTIVITY_CONTROL_TIME;
                fCheckedWrite(&rr, 4, f);
        }
failed:
        if(f) {
                wcfflush(f);
                unlock_file(f);
                wcfclose(f);
        }
        if(f1) {
                wcfflush(f1);
                unlock_file(f1);
                wcfclose(f1);
        }
        return 0;
}
