<p align="center">
	<image src="toylogo.png" />
</p>

**This is a work in progress, and is not yet fit for purpose. I hope I can get it to a useable state, but personal issues can often make dedicating myself to a project difficult. Your patience and support is greatly appreciated.**

# Toy v2.x

The Toy Programming Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through text files.

This repository holds the reference implementation for Toy version 2.x, written in C.

# Nifty Features

* Simple C-style syntax
* Intermediate AST and bytecode representations
* Strong, but optional type system
* First-class functions and closures
* Extensible with imported native code
* Can re-direct output, error and assert failure messages
* Open-Source under the zlib license

# Syntax

Watch this space.

(The `scripts` or `tests` directory might help, the docs website is WIP.)

# Building

This project requires `gcc` and `make` by default, but should also work in other environments. Officially supported platforms include `linux`, `windows` and `macOS`, see `source/toy_common.h` for implementation details.

Run `make` in the root directory to build the shared library named `libToy.so` and a useable REPL named `repl.out`.

# Tools

Watch this space.

(There's some utility functions in `repl/` that are WIP but useful.)

# Documentation

Watch this space.

# License

This source code is covered by the Zlib license (see [LICENSE](LICENSE) for details).

# Contributors and Special Thanks

@NishiOwO - Unofficial NetBSD support  
@Gipson62 - v1 docs spell checking  
@8051Enthusiast - `fixAlignment()` trick  
@hiperiondev - v1 Disassembler, v1 porting support and feedback  
@add00 - v1 Library support  
@gruelingpine185 - Unofficial v1 MacOS support  
@solar-mist - v1 Minor bugfixes  
Various Anons - Feedback  
@munificent - For [writing the book](http://craftinginterpreters.com/) that sparked my interest in langdev  

# Patreon Supporters

You can show your support and be listed here by joining my [Patreon](https://patreon.com/krgamestudios).
