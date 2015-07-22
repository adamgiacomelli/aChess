# common definitions

CC = gcc
PRJCFLAGS = -O3 -std=c99 -I../include -Wall

#.SILENT :

TOP_NAME = Generic
VERSION = 0.0
BD_TEMPLATES = templates/dhmake/

# common rules

all:
	-for d in $(DIRS); do (cd $$d; if [ -f *.pro ]; then qmake; fi; $(MAKE) $(MFLAGS)); done

clean:
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean; if [ -f *.pro ]; then $(RM) Makefile $$d; fi ); done

force_look:
	true

.PHONY: clean doc all

