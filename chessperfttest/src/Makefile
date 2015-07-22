include ../../templates/make/c_common.inc

CFLAGS = $(PRJCFLAGS) -I../../achess/include -L../../achess/lib
OBJ = Achess.o perft.o
EXENAME = perftchess
LIBS = -lachess
OUTDIR = ../bin/

include ../../templates/make/c_executable.inc

install: all
	mkdir -p $(DESTDIR)/usr/bin/
	install $(OUTDIR)/perftchess $(DESTDIR)/usr/bin

.PHONY: install


