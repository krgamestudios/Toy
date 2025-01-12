# Testing Toy

Toy uses GitHub CI/CD for comprehensive automated testing - however, all the tests are under `test/`, and can be executed by running `make test`. Doing so on Linux will attempt to use valgrind; to disable using valgrind, pass in `DISABLE_VALGRIND=true` as an environment variable. GitHub CI also has access to the option `make test-sanitized` which attempts to use memory sanitation.

The tests consist of a number of different situations and edge cases that have been discovered, and should probably be thoroughly tested one way or another. There are also several "-bugfix.toy" scripts that explicitly test a bug that has been encountered in one way or another to prevent regressions. The libs that are stored in `repl/` are also tested - their tests are under `/tests/scripts/lib`; some error cases are also checked by the mustfail tests in `/test/scripts/mustfail`.
