*For the feature complete version 1, [click here](https://github.com/Ratstail91/Toy/tree/v1).*

<p align="center">
	<image src="toylogo.png" />
</p>

# Toy v2.x

The Toy Programming Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through text files.

This repository holds the reference implementation for Toy version 2.x, written in C.

# Nifty Features

* Simple C-like syntax
* Intermediate AST representation
* Strong, but optional type system
* First-class functions and types
* Extensible via external libraries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

# Syntax

```toy
//print is a built-in keyword, that can handle complex expressions
print 6 * 7;

//strings can be concatenated with the .. operator, and substringed with the [] operator
print "Hello" .. "world!"[3, 3]; //[index, length] - this prints "low"

//variables are declared easily
var foobar = 42;

//scopes allow for shadowing and rebinding
{
	var foobar = foobar * 7;
}

//the types default to 'any' but can be specified if needed (same with constants)
var immutable: string const = "Foobar";

//the assert keyword can check an expression, and takes an optional second parameter
assert immutable == "Fizzbuzz", "This message is sent to the terminal by default";

//if and while works
var count = 1;
while (count <= 10) {
	if (count % 2 == 0) {
		print "even";
	}
	else {
		print "odd";
	}
	count += 1;
}

//NOTE: This section will be expanded as more features are implemented
```

# Building

Supported platforms are: `linux-latest`, `windows-latest`, `macos-latest`, using [GitHub's standard runners](https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners/about-github-hosted-runners#standard-github-hosted-runners-for-public-repositories).

To build the shared library, run `make source`.  
To build the shared library and repl, run `make repl`.  
To build and run the standard available tests, run `make tests`.  

# Tools

*Coming Soon, see [#126](https://github.com/Ratstail91/Toy/discussions/126) for details.*

# License

This source code is covered by the zlib license (see [LICENSE.md](LICENSE.md)).

# Contributors and Special Thanks

For a guide on how you can contribute, see [CONTRIBUTING.md](CONTRIBUTING.md).

@8051Enthusiast - `fixAlignment()` trick  
@hiperiondev - v1 Disassembler, v1 porting support and feedback  
@add00 - v1 Library support  
@gruelingpine185 - Unofficial v1 MacOS support  
@solar-mist - v1 Minor bugfixes  
The Ratbags - Feedback  
@munificent - For [writing the book](http://craftinginterpreters.com/) that sparked my interest in langdev

# Patreon Supporters

* Seth A. Robinson


