include ../templates/make/c_common.inc

CFLAGS = $(PRJCFLAGS) -I../achess -I../achess/include -L../achess/lib 
OBJ = echoserv.o helper.o
EXENAME = echoserv
LIBS = -lachess
OUTDIR = bin/

include ../templates/make/c_executable.inc

install: all
	mkdir -p $(DESTDIR)/usr/bin/
	install $(OUTDIR)/chessserver $(DESTDIR)/usr/bin

.PHONY: install
	
force_look:
	true
