<div align="center">
  <image src="toylogo.png" />
</div>

[![Continuous Integration v1.x](https://github.com/krgamestudios/Toy/actions/workflows/continuous-integration-v1.yml/badge.svg)](https://github.com/krgamestudios/Toy/actions/workflows/continuous-integration-v1.yml)  

***Notice**: These docs are valid for version 1.x only - the version 2 docs can be found at [https://toylang.com/](https://toylang.com/).*

# Preamble

The Toy programming language is an imperative bytecode-intermediate embedded scripting language. It isn't intended to operate on its own, but rather as part of another program, the "host". This process is intended to allow a decent amount of easy customisation by the host's end user, by exposing logic in script files. Alternatively, binary files in a custom format can be used as well.

The host will provide all of the extensions needed on a case-by-case basis. Script files have the `.toy` file extension, while binary files have the `.tb` file extension.

The Toy reference implementation can be found on [github](https://github.com/krgamestudios/Toy/tree/v1).

```
fn makeCounter() { //declare a function like this
	var total: int = 0; //declare a variable with a type like this

	fn counter(): int { //declare a return type like this
		return ++total;
	}

	return counter; //closures are explicitly supported
}

var tally = makeCounter();

print tally(); //1
print tally(); //2
print tally(); //3
```

## Nifty Features

* Simple C-like syntax
* Bytecode intermediate compilation
* Optional, but robust type system (including `opaque` for arbitrary data)
* Functions and types are first-class citizens
* Import native libraries from the host
* Fancy slice notation for strings, arrays and dictionaries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

## Getting Started

* [Quick Start Guide](getting-started/quick-start-guide)
* [Types](getting-started/types)
* [Toy Version Info Library](getting-started/toy-version-info-library)
* [Standard Library](getting-started/standard-library)
* [Math Library](getting-started/math.md)
* [Random Library](getting-started/random-library)
* [Runner Library](getting-started/runner-library)

## Deep Dive Document

* [Theorizing Toy](deep-dive/theorizing-toy)
* [Building Toy](deep-dive/building-toy)
* [Embedding Toy](deep-dive/embedding-toy)
* [Compiling Toy](deep-dive/compiling-toy)
* [Developing Toy](deep-dive/developing-toy) (incomplete, but being expanded)
* [Testing Toy](deep-dive/testing-toy)
* [Roadmapping Toy](deep-dive/roadmapping-toy)

## Public C API

* [repl_tools.h](c-api/repl_tools_h.md)
* [drive_system.h](c-api/drive_system_h.md)
* [toy.h](c-api/toy_h.md)
* [toy_common.h](c-api/toy_common_h.md)
* [toy_compiler.h](c-api/toy_compiler_h.md)
* [toy_interpreter.h](c-api/toy_interpreter_h.md)
* [toy_lexer.h](c-api/toy_lexer_h.md)
* [toy_literal_array.h](c-api/toy_literal_array_h.md)
* [toy_literal_dictionary.h](c-api/toy_literal_dictionary_h.md)
* [toy_literal.h](c-api/toy_literal_h.md)
* [toy_memory.h](c-api/toy_memory_h.md)
* [toy_parser.h](c-api/toy_parser_h.md)
* [toy_reffunction.h](c-api/toy_reffunction_h.md)
* [toy_refstring.h](c-api/toy_refstring_h.md)
* [toy_scope.h](c-api/toy_scope_h.md)

## Game Engine

Note: This section is WIP.

* [Game Engine](game-engine/game-engine.md)
* [Box Version Info Library](game-engine/box-version-info-library.md)
* Engine Library
* Node Library
* Input Library
* Music Library
* Sound Library

