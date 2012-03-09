ifeq ($(SRCEXT), cpp)
CC       = $(CXX)
else
CFLAGS  += -std=gnu99
endif


SRCEXT	 = cpp
SRCDIR	 = .
OBJDIR	 = ../obj

SRCS    := $(shell find $(SRCDIR) -maxdepth 1 -name '*.$(SRCEXT)')
OBJS    := $(patsubst %.$(SRCEXT), $(OBJDIR)/%.o, $(SRCS))

DEBUG    = -g
OPTIMALIZATION = -O3
CFLAGS   = -c -Wall $(DEBUG)

.PHONY: all clean


all: $(SRCS) $(OBJS)

$(OBJDIR)/%.o : %.$(SRCEXT)
	@mkdir -p $(OBJDIR)
	@echo "Generating dependencies for $<...";
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@


clean:
	$(RM) -r $(OBJDIR)

# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
  $(CC) -MM       \
        -MF $3    \
        -MP       \
        -MT $2    \
        $(CFLAGS) \
        $1
endef
