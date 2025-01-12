
# Random Library

The random library offers a number of functions geared towards producing pseudorandom values. This library has a concept called "generators", which are opaque objects used to generate a sequence of numbers from an initial integer seed. A seed can be generated from most values using the standard library `hash` function.

The random library can usually be accessed with the `import` keyword:

```
import standard;
import random;

var generator: opaque = createRandomGenerator(clock().hash());
```

The current implementation is minimal in nature, and will be expanded or replaced in future.

## Defined Functions

### createRandomGenerator(seed: int): opaque

This function creates a new generator opaque based on the given seed. The same seed will produce the same sequence of pseudorandom outputs from different generators using `generateRandomNumber`.

Every generator must also be freed with `freeRandomGenerator`.

### generateRandomNumber(self: opaque): int

This function takes in a generator opaque, and returns a pseudorandom integer value.

This function also mutates the generator's internal state.

### freeRandomGenerator(self: opaque)

This function frees an existing generator opaque.

This function must be called on all generators before the program ends.
