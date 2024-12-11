# Test Instructions

To run these tests, execute one of the following commands from the repo's root:

`make tests`  
`make tests-gdb`  
`make tests-valgrind`  

## Benchmarks

For testing and comparing different potential solutions. These may be left in an incomplete state, so they might not work out of the box.

## Cases

For testing individual pieces of the source code in isolation. These are essentially the unit tests, and are used by the CI pipeline.

## Integrations

For testing Toy's processes as a complete whole. This will automatically build the repl, and is used by the CI pipeline.

## Mustfails

These have situations which will raise errors of some kind, to ensure that common user errors are handled gracefully. This is not yet implemented.

## Standalone

These are one-file programs that are not intended to test the source directly. Instead, these can cover a number of situations, such as the exact behavior of GitHub's workflow runners, or to generate repetitive code predictably, etc.

