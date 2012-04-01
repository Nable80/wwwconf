#ifndef DBASEUTILS_H
#define DBASEUTILS_H

#include "basetypes.h"

DWORD Fsize(const char *s);
int ReadDBMessage(DWORD midx, SMessage *mes);
int ReadDBMessageBody(char *buf, DWORD index, DWORD size);
int WriteDBMessage(DWORD midx, SMessage *mes);
int WriteDBMessageBody(char *buf, DWORD index, DWORD size);
DWORD VIndexCountInDB();
DWORD TranslateMsgIndexDel(DWORD root);

#endif
