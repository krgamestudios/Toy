
# Building Toy

This tutorial assumes you're using git, GCC, and make.

To embed toy into your program, simply clone the [git repository](https://github.com/Ratstail91/Toy).

Toy's makefile uses the exported variable `TOY_OUTDIR` to define where the output of the build command will place the result. If you're building Toy as a submodule (which is recommended), then you MUST set this value to a directory name, relative to the root directory.

```make
export TOY_OUTDIR = out
```

Next, you'll want to run make the from within Toy's `source`, assuming the output directory has been created. There are two options for building Toy - `library` (default) or `static`; the former will create a shared library (and a .dll file on Windows), while the latter will create a static library.

```make
toy: $(OUTDIR)
	$(MAKE) -C Toy/source

$(OUTDIR):
	mkdir $(OUTDIR)
```

Finally, link against the outputted library, with the source directory as the location of the header files.

```make
all: $(OBJ) toy
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) -L$(TOY_OUTDIR) -ltoy
```

These snippets of makefile are only an example - the repository has a more fully featured set of makefiles which can also produce a usable REPL program.

