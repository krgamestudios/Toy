#compiler settings reference
#CC=gcc
#CFLAGS+=-std=c17 -g -Wall -Werror -Wextra -Wpedantic -Wformat=2 -Wno-newline-eof
#LIBS+=-lm
#LDFLAGS+=

#TODO: release builds should define the NDEBUG flag; double check it works

#directories
export TOY_SOURCEDIR=source
export TOY_REPLDIR=repl
export TOY_OUTDIR=out
export TOY_OBJDIR=obj

#targets
all: source repl

.PHONY: source
source:
	$(MAKE) -C source -k

.PHONY: repl
repl: source
	$(MAKE) -C repl -k

.PHONY: tests
tests:
	$(MAKE) -C tests -k

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
	find . -type f -name '*.out' -delete
	find . -type f -name '*.exe' -delete
	find . -type f -name '*.dll' -delete
	find . -type f -name '*.lib' -delete
	find . -type f -name '*.so' -delete
	find . -type f -name '*.dylib' -delete
	find . -type d -name 'out' -delete
	find . -type d -name 'obj' -delete
else ifeq ($(shell uname),NetBSD)
	find . -type f -name '*.o' -delete
	find . -type f -name '*.a' -delete
	find . -type f -name '*.out' -delete
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
	find . -type f -name '*.out' -delete
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

