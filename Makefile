#
# Makefile for wwwconf, version by alec@3ka.mipt.ru
# Created 24.01.2003
#

CC=g++
CFLAGS=-O2 -fno-exceptions -Wall -Werror -Wfatal-errors -std=c++0x -Wno-format-contains-nul
.SUFFIXES: .cpp .o
.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp

FILES_INDEX=sha1.cpp statfs.cpp dbase.cpp main.cpp announces.cpp boardtags.cpp speller.cpp security.cpp freedb.cpp indexer.cpp profiles.cpp logins.cpp hashindex.cpp searcher.cpp error.cpp sendmail.cpp colornick.cpp activitylog.cpp
FILES_DBTOOL=freedb.cpp profiles.cpp hashindex.cpp profman.cpp indexer.cpp searcher.cpp
SOURCES=$(FILES_INDEX) $(FILES_DBTOOL)

OBJECTS_INDEX=$(FILES_INDEX:.cpp=.o)
OBJECTS_DBTOOL=$(FILES_DBTOOL:.cpp=.o)

prefix=/home/bbsadmin/board
inst=/usr/bin/install -c

all: index dbtool
	@echo Compiling Done

index: $(OBJECTS_INDEX) .depend
	$(CC) -o index.cgi $(OBJECTS_INDEX)
dbtool: $(OBJECTS_DBTOOL) .depend
	$(CC) -o dbtool $(OBJECTS_DBTOOL)

.depend: $(SOURCES)
	@echo "Creating .depend"
	@rm -f .depend
	$(CC) -MM -MG $(SOURCES) >> .depend

clean:
	@rm -f *.o .depend
	@rm -f index.cgi dbtool 
	@echo Clean complete

install: all
	@echo Installing binaries and tools to ${prefix}
	cp ${prefix}/dbtool ${prefix}/dbtool_prev
	cp ${prefix}/index.cgi ${prefix}/index.cgi_prev
	${inst} -m 751 index.cgi ${prefix}/index.cgi
	${inst} -m 750 dbtool ${prefix}

uninstall: ${prefix}/index.cgi_prev ${prefix}/dbtool_prev 
	@echo UnInstalling backed up binaries and tools 
	cp ${prefix}/index.cgi_prev ${prefix}/index.cgi
	cp ${prefix}/dbtool_prev ${prefix}/dbtool

fake: index 
	@echo Installing binaries and tools to ${prefix}
	${inst} -m 751 index.cgi ${prefix}/index-dev.cgi
	${inst} -m 750 dbtool ${prefix}/dbtool-dev
