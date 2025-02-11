<p align="center">
	<image src="toylogo.png" />
</p>

# Toy v2.x

The Toy Programming Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through text files.

This repository holds the reference implementation for Toy version 2.x, written in C.

# Nifty Features

* Simple C-like/JS-like syntax
* Intermediate AST representation
* Strong, but optional type system
* First-class functions and closures
* Extensible with importable native code
* Can re-direct output, error and assert failure messages
* Open-Source under the Zlib license

# Syntax

The following examples aren't fully functional yet, see [Timetable](#timetable).

```toy
//fizzbuzz example
for (var counter: int = 1; counter <= 100; i++) {
	var result: string = "";

	if (counter % 3 == 0) {
		result = result .. "fizz";
	}

	if (counter % 5 == 0) {
		result = result .. "buzz";
	}

	if (result != "") {
		print result;
	}
	else {
		print counter;
	}
}
```

```toy
//find the nth fibonacci number
fn fib(n: int) {
	if (n < 2) return n;
	return fib(n-1) + fib(n-2);
}

for (var i = 1; i <= 10; i++) {
	print i.toString() .. ":" .. fib(i).toString();
}
```

```toy
//closures!
fn makeCounter() {
	var count = 0;

	fn next() {
		return ++count;
	}

	return next;
}

var tally = makeCounter();

print tally(); //1
print tally(); //2
print tally(); //3
```

# Timetable

Here's a flexible outline for the upcoming feature milestones. On each review date, I'll adjust my projections as needed.

| Feature | Time Span | Review Date |
| --- | :---: | :---: |
| [Arrays & Tables](https://github.com/Ratstail91/Toy/issues/155) | -  | 1st Jan ✅ |
| [Control Flow](https://github.com/Ratstail91/Toy/issues/152) | 2 weeks | 15th Jan ✅ |
| Functions* | ~~2 weeks~~ | ~~29th Jan~~ |
| [Functions](https://github.com/Ratstail91/Toy/issues/163) | 2 weeks | 12th Feb |
| [Dot Operator & Slices](https://github.com/Ratstail91/Toy/issues/156) | 2 weeks | 26th Feb |
| [Native Libraries](https://github.com/Ratstail91/Toy/issues/165) | 2 weeks | 12th Mar |
| [Standard Libraries](https://github.com/Ratstail91/Toy/issues/164) | 2 weeks | 26th Mar |
| [Documentation](https://github.com/Ratstail91/Toy/issues/169) | - | - |

*Info about and strategies for missed milestones can be found on [my blog here](https://krgamestudios.com/posts/2025-01-29-missed-by-a-mile).

# Building

Supported platforms are: `linux-latest`, `windows-latest`, `macos-latest`, using [GitHub's standard runners](https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners/about-github-hosted-runners#standard-github-hosted-runners-for-public-repositories).

Support for NetBSD is present, but not guaranteed.

To build the shared library, run `make source`.  
To build the shared library and repl, run `make repl`.  
To build and run the test suites, run `make tests` (`make tests-gdb` and `make tests-valgrind` options are also available).  

# Tools

*Coming Soon - I want the features mostly working first.*

# Documentation

The WIP documentation can be found here: https://v2.toylang.com/

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

* Seth A. Robinson


