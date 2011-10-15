APP      = fsync_server

SRCEXT   = cpp
SRCDIR   = ./
OBJDIR   = ../../obj/server
COMMONOBJS= ../../obj
BINDIR   = ../../bin

ifeq ($(SRCEXT), cpp)
CC       = $(CXX)
else
CFLAGS  += -std=gnu99
endif

SRCS    := $(shell find $(SRCDIR) -maxdepth 1 -name '*.$(SRCEXT)')
OBJS    := $(patsubst %.$(SRCEXT), $(OBJDIR)/%.o, $(SRCS))

INCLUDES = -I./..
DEBUG    = -g
CFLAGS   = -c -Wall `sdl-config --cflags` $(INCLUDES)
LDFLAGS  = `sdl-config --libs` -lSDL_net -lboost_system -lboost_filesystem \
			$(shell find $(OBJDIR) $(COMMONOBJS) -maxdepth 1 -name '*.o')

.PHONY: all clean distclean

all: $(BINDIR)/$(APP)

$(BINDIR)/$(APP): $(OBJS)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CC) $(LDFLAGS) -o $@


$(OBJDIR)/%.o : %.$(SRCEXT)
	@mkdir -p $(OBJDIR)
	@echo "Generating dependencies for $<...";
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -r $(OBJDIR)

distclean:
	$(RM) $(BINDIR)/$(APP)

# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
  $(CC) -MM       \
        -MF $3    \
        -MP       \
        -MT $2    \
        $(CFLAGS) \
        $1
endef
