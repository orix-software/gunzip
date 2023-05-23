CC=cl65
CFLAGS=-ttelestrat
PROGRAM=gunzip

SOURCE=src/$(PROGRAM).c
LDFILES=

START_ADDR ?= 0x1000

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
	$(CC) -o $(PROGRAM) $(CFLAGS) $(LDFILES) $(SOURCE) --start-addr $(START_ADDR)
	mkdir build/bin -p && mv gunzip build/bin

clean:
	rm -rf build

configure:
	echo configure