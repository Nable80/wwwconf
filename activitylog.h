/***************************************************************************
                          activitylog.h  -  board activity logger header
                             -------------------
    begin                : Mon Oct 6 2003
    copyright            : (C) 2003 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef ACTYVITYLOG_H_INCLUDED
#define ACTYVITYLOG_H_INCLUDED

#include "basetypes.h"

struct SActivityLogRecord
{
        DWORD IP;
        DWORD Count;
        DWORD Time;
};

int RegisterActivityFrom(DWORD IP, DWORD &hostcnt, DWORD &hitcnt);

#endif
