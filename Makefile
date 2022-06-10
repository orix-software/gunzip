CC=cl65
CFLAGS=-ttelestrat
PROGRAM=gunzip
HOMEDIR=/home/travis/bin/
HOMEDIR_PROGRAM=/home/travis/build/orix-software/$(PROGRAM)

SOURCE=src/$(PROGRAM).c
LDFILES=

ifdef TRAVIS_BRANCH
ifeq ($(TRAVIS_BRANCH), master)
RELEASE:=$(shell cat VERSION)
else
RELEASE=alpha
endif
endif


ASFLAGS=-C -W -e error.txt -l xa_labels.txt -DTARGET_ORIX

$(PROGRAM): $(SOURCE)
	$(CC) -o $(PROGRAM) $(CFLAGS) $(LDFILES) $(SOURCE)
	mkdir build/bin -p && mv gunzip build/bin
	mkdir build/usr/share/man -p && cp src/man/gunzip.hlp build/usr/share/man
  
test:
	mkdir -p build/bin/
	mkdir -p build/usr/share/man
	mkdir -p build/usr/share/ipkg  
	mkdir -p build/usr/share/doc/$(PROGRAM)/
	cp $(PROGRAM) build/bin/
	cd $(HOMEDIR) && cat $(HOMEDIR_PROGRAM)/src/man/$(PROGRAM).md | md2hlp.py > $(HOMEDIR_PROGRAM)/build/usr/share/man/$(PROGRAM).hlp  
	cp README.md build/usr/share/doc/$(PROGRAM)/
	cp src/ipkg/$(PROGRAM).csv build/usr/share/ipkg
	cd build &&	tar -c * > ../$(PROGRAM).tar &&	cd ..  
	filepack  $(PROGRAM).tar $(PROGRAM).pkg
	gzip $(PROGRAM).tar
	mv $(PROGRAM).tar.gz $(PROGRAM).tgz
	php buildTestAndRelease/publish/publish2repo.php $(PROGRAM).tgz ${hash} 6502 tgz $(RELEASE)
	echo nothing