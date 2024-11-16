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
TEST_CASESDIR=.

TEST_OUTDIR=out
TEST_OBJDIR=obj

#file names
TEST_SOURCEFILES=$(wildcard $(TEST_SOURCEDIR)/*.c)
TEST_CASESFILES=$(wildcard $(TEST_CASESDIR)/test_*.c)

#build the object files, compile the test cases, and run
all: build-source build-cases build-link build-run

#targets for each step
.PHONY: build-source
build-source: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o)))

.PHONY: build-cases
build-cases: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_CASESFILES:.c=.o)))

.PHONY: build-link
build-link: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe)))

.PHONY: build-run
build-run: $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe))) $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.run)))

#util targets
$(TEST_OUTDIR):
	mkdir $(TEST_OUTDIR)

$(TEST_OBJDIR):
	mkdir $(TEST_OBJDIR)

#compilation steps
$(TEST_OBJDIR)/%.o: $(TEST_SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

$(TEST_OBJDIR)/%.o: $(TEST_CASESDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR) $(TEST_CASESDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

$(TEST_OUTDIR)/%.exe: $(TEST_OBJDIR)/%.o
	@$(CC) -o $@ $< $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o))) $(CFLAGS) $(LIBS) $(LDFLAGS)

.PRECIOUS: $(TEST_OUTDIR)/%.run
$(TEST_OUTDIR)/%.run: $(TEST_OUTDIR)/%.exe
	$<

#debugging targets
gdb: build-source build-cases build-link build-run-gdb

.PHONY: build-run-gdb
build-run-gdb: $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe))) $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.run-gdb)))

.PRECIOUS: $(TEST_OUTDIR)/%.run-gdb
$(TEST_OUTDIR)/%.run-gdb: $(TEST_OUTDIR)/%.exe
	gdb $< -ix gdb_init -ex=run --batch --return-child-result --args "$<"

#valgrind targets
valgrind: build-source build-cases build-link build-run-valgrind

.PHONY: build-run-valgrind
build-run-valgrind: $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe))) $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.run-valgrind)))

.PRECIOUS: $(TEST_OUTDIR)/%.run-valgrind
$(TEST_OUTDIR)/%.run-valgrind: $(TEST_OUTDIR)/%.exe
	valgrind $< --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose
