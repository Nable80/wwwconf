#!/usr/bin/make -f

.PHONY: debug release all

CXX ?= g++
CXXFLAGS = -m32 -fno-exceptions -pedantic $(OPT_FLAGS)
CXXFLAGS += -Wall -Wextra -Werror -Wnarrowing -Wwrite-strings -Wundef

# TODO: enable these options and fix tons of uncovered problems
#CXXFLAGS += -Wcast-qual -Wconversion

ifeq ($(CXX), clang++)
CXXFLAGS += -Wno-invalid-source-encoding -Wno-error=pragma-pack
endif

INDEX_SRCS=statfs.cpp dbaseutils.cpp dbase.cpp main.cpp announces.cpp boardtags.cpp speller.cpp security.cpp freedb.cpp profiles.cpp logins.cpp hashindex.cpp error.cpp sendmail.cpp colornick.cpp activitylog.cpp
DBTOOL_SRCS=error.cpp dbaseutils.cpp freedb.cpp profiles.cpp hashindex.cpp profman.cpp
SRCS=$(INDEX_SRCS) $(DBTOOL_SRCS)

INDEX_OBJS=$(INDEX_SRCS:.cpp=.o)
DBTOOL_OBJS=$(DBTOOL_SRCS:.cpp=.o)

debug: OPT_FLAGS=-ggdb3 -O0
debug: all

release: OPT_FLAGS=-g -O2 -fomit-frame-pointer -flto
release: all

all: index.cgi dbtool
	@echo Compiling Done

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

.depend: $(SRCS)
	@echo "Creating .depend"
	$(CXX) -MM -MG $(SRCS) > .depend

ifneq ($(MAKECMDGOALS), clean)
include .depend
endif

index.cgi: $(INDEX_OBJS) .depend
	$(CXX) $(CXXFLAGS) -o index.cgi -Wl,-\( $(INDEX_OBJS) -Wl,-\)

dbtool: $(DBTOOL_OBJS) .depend
	$(CXX) $(CXXFLAGS) -o dbtool -Wl,-\( $(DBTOOL_OBJS) -Wl,-\)

clean:
	@rm -f *.o .depend
	@rm -f index.cgi dbtool 
	@echo Clean complete
