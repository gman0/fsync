.PHONY: all clean distclean

OBJDIR = obj
BINDIR = bin

all:
	$(MAKE) -C src
	$(MAKE) -C src/client
	$(MAKE) -C src/server

clean:
	$(RM) -r $(OBJDIR)

distclean: clean
	$(RM) -r $(BINDIR)
