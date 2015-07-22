all: $(LIBNAME)

$(LIBNAME): $(OBJ) $(OUTDIR)
	ar r $(OUTDIR)$(LIBNAME) $(OBJ)

$(OUTDIR):
	if [ ! -d $(OUTDIR) ]; then mkdir $(OUTDIR); fi

clean:
	rm -rf $(LIBNAME) *.o *~ $(OUTDIR)

.PHONY: clean all
