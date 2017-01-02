#
# Makefile for wwwconf, version by alec@3ka.mipt.ru
# Created 24.01.2003
#

.PHONY: debug

debug: all

CC ?= g++
ifeq ($(MAKECMDGOALS), debug)
CFLAGS=-m32 -ggdb3 -O0 -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -pedantic -Wno-invalid-source-encoding
else
CFLAGS=-m32 -O2 -fomit-frame-pointer -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -pedantic -Wno-invalid-source-encoding
endif

INDEX_SRCS=statfs.cpp dbaseutils.cpp dbase.cpp main.cpp announces.cpp boardtags.cpp speller.cpp security.cpp freedb.cpp profiles.cpp logins.cpp hashindex.cpp error.cpp sendmail.cpp colornick.cpp activitylog.cpp
DBTOOL_SRCS=error.cpp dbaseutils.cpp freedb.cpp profiles.cpp hashindex.cpp profman.cpp
SRCS=$(INDEX_SRCS) $(DBTOOL_SRCS)

INDEX_OBJS=$(INDEX_SRCS:.cpp=.o)
DBTOOL_OBJS=$(DBTOOL_SRCS:.cpp=.o)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp

.depend: $(SRCS)
	@echo "Creating .depend"
	$(CC) -MM -MG $(SRCS) > .depend

ifneq ($(MAKECMDGOALS), clean)
include .depend
endif

all: index.cgi dbtool
	@echo Compiling Done

index.cgi: $(INDEX_OBJS) .depend
	$(CC) $(CFLAGS) -o index.cgi -Wl,-\( $(INDEX_OBJS) -Wl,-\)

dbtool: $(DBTOOL_OBJS) .depend
	$(CC) $(CFLAGS) -o dbtool -Wl,-\( $(DBTOOL_OBJS) -Wl,-\)

clean:
	@rm -f *.o .depend
	@rm -f index.cgi dbtool 
	@echo Clean complete
