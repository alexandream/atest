**Disclaimer:** This is still a work in progress. You should not be using
this.

**Disclaimer:** Yeah, the README is also a work in progress ;)


# atest

A unit testing library for C programs.

Designed with simplicity and compatibility in mind, the library adheres to
the C89 standard, avoiding any compiler extensions. Due to that reason,
while I find it useful to work with, if you're looking for fancy features
like auto registration of tests or isolating tests in process to capture
errors.

These features, if ever, will only be available as optional modules if I end
up feeling the need for it in my own projects or if some kind soul makes a
pull request with it ;)


## Getting Started

To get up and running with it just get the latest version and run `make`.
For now it builds using a simple hand crafted Makefile but I'm considering
migrating to CMake further down the road.

After running `make` two files will be available in the `dist/` directory:
`libatest.a` and `atest.h`. These are all you'll need to start using this
library, albeit admittedly with some unneeded work.


### Prerequisites

Basically all you need is a C89 compatible compiler and a flavour of Make.


## License

This project is licensed under the BSD 3-Clause License - see the
[LICENSE.md](LICENSE.md) file for details

