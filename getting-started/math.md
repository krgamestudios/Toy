# Math Library

The math library is a collection of mathematical functions and constants that provide a wide range of calculations that are commonly used. All functions in this library take either integers or floats as parameters and will return the result as a float.

The math library can usually be accessed with the `import` keyword:

```
import math;
```

## Defined Constants

### PI: float

This constant represents the ratio of a circle's circumference to its diameter. Its value is approximately `3.14159265358979323846`.

### E: float

This constant represents Euler's number, the base of natural logarithms. Its value is approximately `2.71828182845904523536`.

### EPSILON: float

This constant represents the acceptable amount of error when comparing floats with the functions provided by this library (see [Defined Comparison Functions](#defined-comparison-functions)). Its default value is `0.000001`.

### NAN: float

This constant represents "Not-a-Number", often returned when a calculation is impossible, e.g. `sqrt(-1)`.

### INFINITY: float

This constant represents an uncountable value.

## Defined Power Functions

### pow(x, y): float

This function returns `x` to the power of `y`.

### sqrt(x): float

This function returns the square root of `x`.

### qbrt(x): float

This function returns the cube root of `x`.

### hypot(x, y): float

This function returns the length of the hypotenuse, assuming `x` and `y` are the legs in a right-angle triangle.

## Defined Trigonometric Functions

### toRadians(d): float

This function converts `d` into radians.

### toDegrees(r): float

This function converts `r` into degrees.

### sin(x): float

This function returns the sine of `x`.

### cos(x): float

This function returns the cosine of `x`.

### tan(x): float

This function returns the tangent of `x`.

### asin(x): float

This function returns the arc sine of `x`.

### acos(x): float

This function returns the arc cosine of `x`.

### atan(x): float

This function returns the arc tangent of `x`.

## Defined Hyperbolic Functions

### sinh(x): float

This function returns the hyperbolic sine of `x`

### cosh(x): float

This function returns the hyperbolic cosine of `x`

### tanh(x): float

This function returns the hyperbolic tangent of `x`

### asinh(x): float

This function returns the inverse hyperbolic sine of `x`

### acosh(x): float

This function returns the inverse hyperbolic cosine of `x`

### atanh(x): float

This function returns the inverse hyperbolic tangent of `x`

## Defined Comparison Functions

### checkIsNaN(x): bool

This function returns true if `x` is NaN, otherwise it returns false.

### checkIsFinite(x): bool

This function returns true if `x` is finite, otherwise it returns false.

### checkIsInfinite(x): bool

This function returns true if `x` is Infinite, otherwise it returns false.

### epsilionCompare(x, y): bool

This function returns true if `x` and `y` are within `EPSILON` of each other, otherwise it returns false. This is very useful for comparing floating point values.
