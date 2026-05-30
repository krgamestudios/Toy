#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wextra -Wpedantic -Wformat=2 -Wno-newline-eof
LIBS+=-lm
LDFLAGS+=

ifeq ($(shell uname),Linux)
LDFLAGS=-Wl,--gc-sections
else ifeq ($(shell uname),NetBSD)
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
TEST_SCRIPTDIRS=basics values keywords algorithms

#file names
TEST_SCRIPTFILES=$(foreach dir,$(TEST_SCRIPTDIRS), $(wildcard $(dir)/test_*.toy))

#build the source and repl, copy to the local dir, and run
all: source repl copy run

#compile the source and repl first
source:
	$(MAKE) -C $(TEST_SOURCEDIR)

repl: source
	$(MAKE) -C $(TEST_REPLDIR)

copy:
	cp -r $(TEST_ROOTDIR)/$(TOY_OUTDIR) .

run: $(TEST_SCRIPTFILES:.toy=.toy-run)

%.toy-run: %.toy
	find . -name repl* -type f -exec {} -f $< \;

#using gdb
gdb: source repl copy run-gdb

run-gdb: $(TEST_SCRIPTFILES:.toy=.toy-run-gdb)

%.toy-run-gdb: %.toy
	gdb $(TEST_OUTDIR)/$(TEST_REPLNAME) -ix gdb_init -ex=run --batch --return-child-result --args "out/repl.out" "-f" "$<"