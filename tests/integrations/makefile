#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wformat=2
LIBS+=-lm
LDFLAGS+=

ifeq ($(shell uname),Linux)
LDFLAGS=-Wl,--gc-sections
else ifeq ($(OS),Windows_NT)
LDFLAGS=-Wl,--gc-sections
else ifeq ($(shell uname),Darwin)
LDFLAGS=-Wl,-dead_strip
else
	@echo "LDFLAGS set failed - what platform is this?"
endif

#directories
TEST_ROOTDIR=../..
TEST_SOURCEDIR=$(TEST_ROOTDIR)/$(TOY_SOURCEDIR)
TEST_REPLDIR=$(TEST_ROOTDIR)/$(TOY_REPLDIR)
TEST_SCRIPTDIR=.

TEST_OUTDIR=out
TEST_OBJDIR=obj

#file names
TEST_SCRIPTFILES=$(wildcard $(TEST_SCRIPTDIR)/test_*.toy)
TEST_REPLNAME=repl.exe

#build the source and repl, and run
all: source repl run

run: $(TEST_SCRIPTFILES:.toy=.toy-run)

%.toy-run: %.toy
	$(TEST_OUTDIR)/$(TEST_REPLNAME) -f ../$< --verbose

#same as above, but with gdb
gdb: source repl run-gdb

run-gdb: $(TEST_SCRIPTFILES:.toy=.toy-gdb)

%.toy-gdb: %.toy
	gdb $(TEST_OUTDIR)/$(TEST_REPLNAME) -ix gdb_init -ex=run --batch --return-child-result --args "$(TEST_OUTDIR)/$(TEST_REPLNAME)" "-f" "../$<" "--verbose"

#same as above, but with valgrind
valgrind: source repl run-valgrind

run-valgrind: $(TEST_SCRIPTFILES:.toy=.toy-valgrind)

%.toy-valgrind: %.toy
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes $(TEST_OUTDIR)/$(TEST_REPLNAME) -f ../$< --verbose

#compile the source and repl first
source: $(TEST_OBJDIR) $(TEST_OUTDIR)
	$(MAKE) SRC_OUTDIR=../$(TOY_INTEGRATIONSDIR)/$(TEST_OUTDIR) -C $(TEST_SOURCEDIR)

repl: $(TEST_OBJDIR) $(TEST_OUTDIR) source
	$(MAKE) REPL_TARGETNAME=$(TEST_REPLNAME) REPL_OUTDIR=../$(TOY_INTEGRATIONSDIR)/$(TEST_OUTDIR) -C $(TEST_REPLDIR)

#util targets
$(TEST_OUTDIR):
	mkdir $(TEST_OUTDIR)

$(TEST_OBJDIR):
	mkdir $(TEST_OBJDIR)
