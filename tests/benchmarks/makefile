#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wextra -Wformat=2
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

#patched in
TOY_SOURCEDIR=source

#directories
TEST_ROOTDIR=../..
TEST_SOURCEDIR=$(TEST_ROOTDIR)/$(TOY_SOURCEDIR)
TEST_CASESDIR=.

TEST_OUTDIR=out
TEST_OBJDIR=obj

#file names
TEST_SOURCEFILES=$(wildcard $(TEST_SOURCEDIR)/*.c)
TEST_CASESFILES=$(wildcard $(TEST_CASESDIR)/bench_*.c)

#build the object files, compile the test cases, and run
all: clean
	$(MAKE) build-source
	$(MAKE) build-cases
	$(MAKE) build-link
	$(MAKE) build-run

all-override: clean
	$(MAKE) TEST_SOURCEFILES='$(subst $(TEST_SOURCEDIR)/$(OVERRIDE),$(OVERRIDE),$(TEST_SOURCEFILES))' build-source-override
	$(MAKE) build-cases
	$(MAKE) build-link
	$(MAKE) build-run

.PHONY: build-source-override
build-source-override: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o)))
	$(CC) -c -o $(TEST_OBJDIR)/$(OVERRIDE:.c=.o) $(OVERRIDE) $(addprefix -I,$(TEST_SOURCEDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

#targets for each step
.PHONY: build-source
build-source: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o)))

.PHONY: build-cases
build-cases: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_CASESFILES:.c=.o)))

.PHONY: build-link
build-link: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe)))

.PHONY: build-run
build-run: $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe))) $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.run)))

#compilation steps
$(TEST_OBJDIR)/%.o: $(TEST_SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

$(TEST_OBJDIR)/%.o: $(TEST_CASESDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR) $(TEST_CASESDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

$(TEST_OUTDIR)/%.exe: $(TEST_OBJDIR)/%.o
	@$(CC) -o $@ $< $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o))) $(CFLAGS) $(LIBS) $(LDFLAGS)

.PRECIOUS: $(TEST_OUTDIR)/%.run
$(TEST_OUTDIR)/%.run: $(TEST_OUTDIR)/%.exe
	@/usr/bin/time --format "%C; $(OVERRIDE)\nUser System\n%U %E" $< 100000000 512
	@/usr/bin/time --format "%C; $(OVERRIDE)\nUser System\n%U %E" $< 100000000 1024
	@/usr/bin/time --format "%C; $(OVERRIDE)\nUser System\n%U %E" $< 100000000 4096

#util targets
$(TEST_OUTDIR):
	mkdir $(TEST_OUTDIR)

$(TEST_OBJDIR):
	mkdir $(TEST_OBJDIR)

#util commands
.PHONY: clean
clean:
ifeq ($(shell uname),Linux)
	find . -type f -name '*.o' -delete
	find . -type f -name '*.a' -delete
	find . -type f -name '*.exe' -delete
	find . -type f -name '*.dll' -delete
	find . -type f -name '*.lib' -delete
	find . -type f -name '*.so' -delete
	find . -type f -name '*.dylib' -delete
	find . -type d -name 'out' -delete
	find . -type d -name 'obj' -delete
else ifeq ($(OS),Windows_NT)
	$(RM) *.o *.a *.exe *.dll *.lib *.so *.dylib
	$(RM) out
	$(RM) obj
else ifeq ($(shell uname),Darwin)
	find . -type f -name '*.o' -delete
	find . -type f -name '*.a' -delete
	find . -type f -name '*.exe' -delete
	find . -type f -name '*.dll' -delete
	find . -type f -name '*.lib' -delete
	find . -type f -name '*.so' -delete
	find . -type f -name '*.dylib' -delete
	find . -type d -name 'out' -delete
	find . -type d -name 'obj' -delete
else
	@echo "Deletion failed - what platform is this?"
endif

