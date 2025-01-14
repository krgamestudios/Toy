#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wextra -Wpedantic -Wformat=2 -Wno-newline-eof
LIBS+=-lm -lToy
LDFLAGS+=-Wl,-rpath,'$$ORIGIN'

#directories
REPL_ROOTDIR=..
REPL_REPLDIR=.
REPL_SOURCEDIR=$(REPL_ROOTDIR)/source

REPL_OUTDIR=$(REPL_ROOTDIR)/out
REPL_OBJDIR=obj

#file names
REPL_REPLFILES=$(wildcard $(REPL_REPLDIR)/*.c)
REPL_OBJFILES=$(addprefix $(REPL_OBJDIR)/,$(notdir $(REPL_REPLFILES:.c=.o)))
REPL_TARGETNAME=repl.exe

#linker fix
LDFLAGS+=-L$(realpath $(REPL_OUTDIR))

#build the object files, compile the test cases, and run
all: build link

#targets for each step
.PHONY: build
build: $(REPL_OBJDIR) $(REPL_OBJFILES)

.PHONY: link
link: $(REPL_OUTDIR) $(REPL_OUTDIR)/$(REPL_TARGETNAME)

#util targets
$(REPL_OUTDIR):
	mkdir $(REPL_OUTDIR)

$(REPL_OBJDIR):
	mkdir $(REPL_OBJDIR)

#compilation steps
$(REPL_OBJDIR)/%.o: $(REPL_REPLDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(REPL_REPLDIR)) $(addprefix -I,$(REPL_SOURCEDIR)) $(CFLAGS)

$(REPL_OUTDIR)/$(REPL_TARGETNAME): $(REPL_OBJFILES)
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $@ $(REPL_OBJFILES) $(LDFLAGS) $(LIBS)
ifeq ($(shell uname),Darwin) #dylib fix
	otool -L $@
	install_name_tool -add_rpath @executable_path/. $@
	install_name_tool -change $(REPL_OUTDIR)/libToy.dylib @executable_path/libToy.dylib $@
	otool -L $@
endif
