CC=cl65
CFLAGS=-ttelestrat
PROGRAM=gunzip

SOURCE=src/$(PROGRAM).c
LDFILES=

ifeq ($(CC65_HOME),)
        CC = cl65
        AS = ca65
        LD = ld65
        AR = ar65
else
        CC = $(CC65_HOME)/bin/cl65
        AS = $(CC65_HOME)/bin/ca65
        LD = $(CC65_HOME)/bin/ld65
        AR = $(CC65_HOME)/bin/ar65
endif

$(PROGRAM): $(SOURCE)
	$(CC) -o $(PROGRAM) $(CFLAGS) $(LDFILES) $(SOURCE)
	mkdir build/bin -p && mv gunzip build/bin
	mkdir build/usr/share/man -p && cp src/man/gunzip.hlp build/usr/share/man

