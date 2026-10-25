<p align="center">
	<image src="img/toylogo.png" alt="The Toy Logo" />
</p>

The Toy Scripting Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through external scripts.

## Nifty Features

* Simple C-like syntax
* Intermediate AST and bytecode representations
* Strong, but optional type system
* First-class functions and closures
* Extensible with native C-bindings
* Can re-direct output, error and assertion messages
* Open-Source under the zlib license

## Syntax

```toy
fn makeCounter() {
	var counter: Int = 0;

	fn increment() {
		return ++counter;
	}

	return increment;
}

//'tally' holds internal state
var tally = makeCounter();

for (var result = 0; result < 10; result = tally()) {
	print result;
}
```

## Toy In Action

I'm currently developing a game using Toy as a primary component, which can be found here:

[https://github.com/krgamestudios/ToyBox](https://github.com/krgamestudios/ToyBox)

## Further Reading

This website is a work in progress - please be patient!

<p align="center">
	<image src="img/noai.png" alt="No AI" width="200px" />
</p>
