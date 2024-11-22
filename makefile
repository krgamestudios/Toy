#compiler settings reference
#CC=gcc
#CFLAGS+=-std=c17 -g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wformat=2
#LIBS+=-lm
#LDFLAGS+=

#directories
export TOY_SOURCEDIR=source
export TOY_REPLDIR=repl
export TOY_CASESDIR=tests/cases
export TOY_INTEGRATIONSDIR=tests/integrations
export TOY_OUTDIR=out
export TOY_OBJDIR=obj

#targets
#all:

.PHONY: source
source:
	$(MAKE) -C source -k

.PHONY: repl
repl: source
	$(MAKE) -C repl -k

#various kinds of available tests
.PHONY: tests
tests: clean test-cases test-integrations

.PHONY: test-cases
test-cases:
	$(MAKE) -C $(TOY_CASESDIR) -k

.PHONY: test-integrations
test-integrations:
	$(MAKE) -C $(TOY_INTEGRATIONSDIR) -k

#same as above, but with GDB
.PHONY: tests-gdb
tests-gdb: clean test-cases-gdb test-integrations-gdb

.PHONY: test-cases-gdb
test-cases-gdb:
	$(MAKE) -C $(TOY_CASESDIR) gdb -k

.PHONY: test-integrations-gdb
test-integrations-gdb:
	$(MAKE) -C $(TOY_INTEGRATIONSDIR) gdb -k

#same as above, but with valgrind
.PHONY: tests-valgrind
tests-valgrind: clean test-cases-valgrind test-integrations-valgrind

.PHONY: test-cases-valgrind
test-cases-valgrind:
	$(MAKE) -C $(TOY_CASESDIR) valgrind -k

.PHONY: test-integrations-valgrind
test-integrations-valgrind:
	$(MAKE) -C $(TOY_INTEGRATIONSDIR) valgrind -k

#Run all tests
.PHONY: tests-all
tests-all: clean tests tests-gdb tests-valgrind

#TODO: mustfail tests

#util targets
$(TOY_OUTDIR):
	mkdir $(TOY_OUTDIR)

$(TOY_OBJDIR):
	mkdir $(TOY_OBJDIR)

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

