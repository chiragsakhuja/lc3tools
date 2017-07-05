# lc3v2 [![Build Status](https://travis-ci.org/chiragsakhuja/lc3v2.svg?branch=master)](https://travis-ci.org/chiragsakhuja/lc3v2)
`lc3v2` is a complete overhaul of the current LC-3 simulator used in EE 306 at
The University of Texas at Austin.  With `lc3v2`, we strive to achieve the
following goals:
* Create a truly cross platform assembler and simulator.
* Design an interface that unifies editing, assembling, and simulating code.
* Develop a formal testing infrastructure that can be used with equal ease by
  both students and graders.
* Integrate a submission system with the interface to encourage frequent
  submissions and/or version control.
* Maintain an organized, easily extendable, and open source code base.

Note on cross-platform compatibility: The two major problems with the existing
simulator are 1) that the code base has fragmented on Windows and Linux such that
different functionality is supported, and 2) OS X has no official support. The goal
of `lc3v2` to be truly cross platform implies that not only will complete
functionality be supported on all major platforms, but also that the code should
be compileable on these platforms.


# Building
`lc3v2` uses CMake for the build system and the only packages it relies
on outside of the source tree are Flex and Bison. On a Linux or OS X
system, you can execute the following commands to compile the project.

```
# create build directory
mkdir build && cd build
# set up build directory
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake ..
# build all the targets
make all
```

Building the project will generate a `bin` directory inside the `build`
directory that contains the targets.

# Usage
Currently the following executables are produced upon building:
* `lib/libjsonxx.so` is a third-party JSON parser.
* `lib/liblc3core.so` is the backend for the assembler and simulator.
* `bin/assembler` is a CLI frontend for the assembler.

## assembler
`assembler` takes assembly files as inputs and generates assembled files with the
same name. For example, runnning `assembler proj1.asm` will generate the files
`proj1.bin`, `proj1.hex`, and `proj1.obj`.

# Testing
`lc3v2` uses Google Test to test the individual components. Google Test
is downloaded as part of the build process. To run the tests, you must
run the following commands, after the project is initially built.
```
# set up build directory
cmake -Dtest=on -DPRINT_LEVEL=0 ..
cmake ..
# build testing targets
make all
# run testing suite
make test
```
**Note:** Currently the testing framework only works on UNIX based
platforms (i.e. it does not work on Windows).
