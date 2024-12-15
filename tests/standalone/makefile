#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wextra -Wpedantic -Wformat=2
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
TEST_SRCDIR=

TEST_OUTDIR=out/
TEST_OBJDIR=obj/

#file names
TEST_SRCFILES=$(wildcard $(TEST_SRCDIR)*.c)

#kick off
all: $(TEST_OBJDIR) $(TEST_OUTDIR) build run

gdb: $(TEST_OBJDIR) $(TEST_OUTDIR) build gdb-run

#build
build: $(TEST_OBJDIR)$(TEST_SRCFILES:.c=.o)

.PRECIOUS: $(TEST_OBJDIR)%.o
$(TEST_OBJDIR)%.o: $(TEST_SRCDIR)%.c
	$(CC) -c -o $@ $< $(CFLAGS) -fdata-sections -ffunction-sections

.PRECIOUS: $(TEST_OUTDIR)%.exe
$(TEST_OUTDIR)%.exe: $(TEST_OBJDIR)%.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(LDFLAGS)

#run
run: $(addprefix $(TEST_OUTDIR),$(TEST_SRCFILES:.c=.exe-run))

$(TEST_OUTDIR)%.exe-run: $(TEST_OUTDIR)%.exe
	$<

#gdb-run
gdb-run: $(addprefix $(TEST_OUTDIR),$(TEST_SRCFILES:.c=.exe-gdb-run))

$(TEST_OUTDIR)%.exe-gdb-run: $(TEST_OUTDIR)%.exe
	gdb $< -ix gdb_init -ex=run --batch --return-child-result

#util targets
$(TEST_OUTDIR):
	mkdir $(TEST_OUTDIR)

$(TEST_OBJDIR):
	mkdir $(TEST_OBJDIR)
