#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wextra -Wformat=2
LIBS+=-lm
LDFLAGS+=

#directories
SRC_ROOTDIR=..
SRC_SOURCEDIR=.

SRC_OUTDIR=$(SRC_ROOTDIR)/out
SRC_OBJDIR=obj

#file names
SRC_SOURCEFILES=$(wildcard $(SRC_SOURCEDIR)/*.c)
SRC_OBJFILES=$(addprefix $(SRC_OBJDIR)/,$(notdir $(SRC_SOURCEFILES:.c=.o)))
SRC_TARGETNAME=Toy

#SRC_LIBLINE is a fancy way of making the linker work correctly
ifeq ($(shell uname),Linux)
	SRC_TARGETEXT=.so
	SRC_LIBLINE=-shared -Wl,-rpath,. -Wl,--out-implib=$(SRC_OUTDIR)/lib$(SRC_TARGETNAME).a -Wl,--whole-archive $(SRC_OBJFILES) -Wl,--no-whole-archive
	CFLAGS+=-fPIC
else ifeq ($(OS),Windows_NT)
	SRC_TARGETEXT=.dll
	SRC_LIBLINE=-shared -Wl,-rpath,. -Wl,--out-implib=$(SRC_OUTDIR)/lib$(SRC_TARGETNAME).a -Wl,--whole-archive $(SRC_OBJFILES) -Wl,--no-whole-archive -Wl,--export-all-symbols -Wl,--enable-auto-import
else ifeq ($(shell uname),Darwin)
	SRC_TARGETEXT=.dylib
	SRC_LIBLINE=-shared -Wl,-rpath,.  $(SRC_OBJFILES)
else
	@echo "Platform test failed - what platform is this?"
	exit 1
endif

#build the object files, compile the test cases, and run
all: build link

#targets for each step
.PHONY: build
build: $(SRC_OUTDIR) $(SRC_OBJDIR) $(SRC_OBJFILES)

.PHONY: link
link: $(SRC_OUTDIR)
	$(CC) -DTOY_EXPORT $(CFLAGS) -o $(SRC_OUTDIR)/lib$(SRC_TARGETNAME)$(SRC_TARGETEXT) $(SRC_LIBLINE)

#util targets
$(SRC_OUTDIR):
	mkdir $(SRC_OUTDIR)

$(SRC_OBJDIR):
	mkdir $(SRC_OBJDIR)

#compilation steps
$(SRC_OBJDIR)/%.o: $(SRC_SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(SRC_SOURCEDIR)) $(CFLAGS)	
