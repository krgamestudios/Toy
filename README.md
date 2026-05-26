<p align="center">
	<image src="toylogo.png" alt="The Toy Logo" />
</p>

# Toy v2.x

The Toy Programming Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through external scripts.

This repository holds the reference implementation for Toy version 2.x, written in C - open beta is currently underway.

# Nifty Features

* Simple C-like syntax
* Intermediate AST and bytecode representations
* Strong, but optional type system
* First-class functions and closures
* Extensible with native C-bindings
* Can re-direct output, error and assertion messages
* Open-Source under the zlib license

# Syntax

```toy
fn makeCounter() {
	var counter: Int = 0;

	fn increment() {
		return ++counter;
	}

	return increment;
}

var tally = makeCounter();

var result = 0;
while (result = tally()) {
	print result;

	if (result >= 10) {
		break;
	}
}
```

# Building

This project requires `gcc` and `make` by default, but should also work in other environments. Officially supported platforms include `linux`, `windows` and `macOS`, see `source/toy_common.h` for implementation details.

Run `make` in the root directory to build the shared library named `libToy.so` and a useable REPL named `repl.out`.

# Documentation

The contents of `docs/` is also available on the official website [toylang.com](https://toylang.com/).

# License

This source code is covered by the Zlib license (see [LICENSE](LICENSE) for details).

# Contributors and Special Thanks

<p align="center">
	<image src="noai.png" alt="No AI" width="200px" />
</p>

Contributions via the [GitHub mirror](https://github.com/krgamestudios/Toy) are welcome, but absolutely no AI contributions will be accepted.

"No AI" logo by [Suyash Dwivedi, via wikimedia, CC BY-SA 4.0](https://commons.wikimedia.org/w/index.php?curid=165477595)  
@NishiOwO - Unofficial NetBSD support  
@Gipson62 - v1 docs spell checking  
@8051Enthusiast - `fixAlignment()` trick  
@hiperiondev - v1 Disassembler, v1 porting support and feedback  
@add00 - v1 Library support  
@gruelingpine185 - Unofficial v1 MacOS support  
@solar-mist - v1 Minor bugfixes  
Various Anons - Feedback  
@munificent - For [writing the book](http://craftinginterpreters.com/) that sparked my interest in langdev  

# Financial Supporters

You can show your support and be listed here by joining my [Patreon](https://patreon.com/krgamestudios), or buying me a coffee at [ko-fi](https://ko-fi.com/krgamestudios).