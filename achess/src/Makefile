include ../../templates/make/c_common.inc

CFLAGS += $(PRJCFLAGS)
OBJ = logic.o
LIBNAME = libachess.a
DESTDIR = 
OUTDIR = ../lib/

include ../../templates/make/c_static_lib.inc

install: all
	mkdir -p $(DESTDIR)/usr/include/achess
	mkdir -p $(DESTDIR)/usr/lib
	install $(OUTDIR)/libachess.a $(DESTDIR)/usr/lib
	install ../include/enums.h $(DESTDIR)/usr/include/achess
	install ../include/logic.h $(DESTDIR)/usr/include/achess

.PHONY: clean all install

force_look:
	true

