CC=cl65
CFLAGS=-ttelestrat
PROGRAM=gunzip

all: build docs package



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

build:
	mkdir build/usr/share/man -p
	mkdir build/bin -p
	#$(CC) -o $(PROGRAM) $(CFLAGS) $(LDFILES) $(SOURCE) --start-addr \$800
	$(CC) -o 1000 $(CFLAGS) $(LDFILES) $(SOURCE) --start-addr 0x800
	$(CC) -o 1256 $(CFLAGS) $(LDFILES) $(SOURCE) --start-addr 0x900
	# Reloc
	chmod +x dependencies/orix-sdk/bin/relocbin.py3
	dependencies/orix-sdk/bin/relocbin.py3 -o build/bin/gunzip -2 1000 1256
	cd docs && ../../md2hlp/src/md2hlp.py3 --file gunzip.md --output ../build/usr/share/man/gunzip.hlp && cd ..

doc:
	cd docs/ && sh builddocs.sh && cd ..

clean:
	rm -rf build

configure:
	echo configure

package:
	cp build/* package/ -r
	cd package/ && dpkg-deb --build -Zgzip --nocheck gunzip && cd ..
