all: $(EXENAME)

$(EXENAME): $(OBJ) $(OUTDIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUTDIR)$@ $(LIBS)

$(OUTDIR):
	if [ ! -d $(OUTDIR) ]; then mkdir $(OUTDIR); fi

clean:
	rm -rf $(EXENAME).exe $(EXENAME) *.o *~ $(OUTDIR)

.PHONY: clean all
