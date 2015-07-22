include ../../templates/make/c_common.inc


CFLAGS = $(PRJCFLAGS) -I../../achess -I../../achess/include -L../../achess/lib 
OBJ = Achess.o ui_ascii.o
EXENAME = asciiuichess
LIBS = -lachess
OUTDIR = ../bin/

include ../../templates/make/c_executable.inc

install: all
	mkdir -p $(DESTDIR)/usr/bin/
	install $(OUTDIR)/asciiuichess $(DESTDIR)/usr/bin

.PHONY: install
	
force_look:
	true
