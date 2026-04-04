<p align="center">
	<image src="toylogo.png" />
</p>

<p style="color:red;font-weight: bold;">This is a work in progress, and is not fit for purpose. I hope I can get it to a useable state, but personal issues can often make dedicating myself to a project difficult. Your patience and support is greatly appreciated.</p>

# Toy v2.x

The Toy Programming Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through text files.

This repository holds the reference implementation for Toy version 2.x, written in C.

# Nifty Features

* Simple C-like/JS-like syntax
* Intermediate AST representation
* Strong, but optional type system
* First-class functions and closures
* Extensible with imported native code
* Can re-direct output, error and assert failure messages
* Open-Source under the zlib license

# Syntax

Watch this space.

(The `scripts` or `tests` directory might help.)

# Building

TODO: Look into cmake

Supported platforms are: `linux-latest`, `windows-latest`, `macos-latest`, using [GitHub's standard runners](https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners/about-github-hosted-runners#standard-github-hosted-runners-for-public-repositories).

Support for NetBSD is present, but not guaranteed.

To build the shared library, run `make source`.  
To build the shared library and repl, run `make repl`.  
To build and run the test suites, run `make tests` (`make tests-gdb` and `make tests-valgrind` options are also available).  

# Tools

Watch this space.

# Documentation

Watch this space.

# License

This source code is covered by the Zlib license (see [LICENSE.md](LICENSE.md)).

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

Watch this space.

You can show your support and be listed here by joining my [Patreon](https://patreon.com/krgamestudios).
