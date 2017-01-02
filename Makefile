#
# Makefile for wwwconf, version by alec@3ka.mipt.ru
# Created 24.01.2003
#

.PHONY: debug

debug: all

CXX ?= g++
ifeq ($(MAKECMDGOALS), debug)
CXXFLAGS=-m32 -ggdb3 -O0 -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -pedantic -Wno-invalid-source-encoding
else
CXXFLAGS=-m32 -O2 -fomit-frame-pointer -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -pedantic -Wno-invalid-source-encoding
endif

INDEX_SRCS=statfs.cpp dbaseutils.cpp dbase.cpp main.cpp announces.cpp boardtags.cpp speller.cpp security.cpp freedb.cpp profiles.cpp logins.cpp hashindex.cpp error.cpp sendmail.cpp colornick.cpp activitylog.cpp
DBTOOL_SRCS=error.cpp dbaseutils.cpp freedb.cpp profiles.cpp hashindex.cpp profman.cpp
SRCS=$(INDEX_SRCS) $(DBTOOL_SRCS)

INDEX_OBJS=$(INDEX_SRCS:.cpp=.o)
DBTOOL_OBJS=$(DBTOOL_SRCS:.cpp=.o)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

.depend: $(SRCS)
	@echo "Creating .depend"
	$(CXX) -MM -MG $(SRCS) > .depend

ifneq ($(MAKECMDGOALS), clean)
include .depend
endif

all: index.cgi dbtool
	@echo Compiling Done

index.cgi: $(INDEX_OBJS) .depend
	$(CXX) $(CXXFLAGS) -o index.cgi -Wl,-\( $(INDEX_OBJS) -Wl,-\)

dbtool: $(DBTOOL_OBJS) .depend
	$(CXX) $(CXXFLAGS) -o dbtool -Wl,-\( $(DBTOOL_OBJS) -Wl,-\)

clean:
	@rm -f *.o .depend
	@rm -f index.cgi dbtool 
	@echo Clean complete
