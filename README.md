# lc3tools [![Build Status](https://travis-ci.org/chiragsakhuja/lc3tools.svg?branch=master)](https://travis-ci.org/chiragsakhuja/lc3tools)
`lc3tools` is a complete overhaul of the current LC-3 simulator used alongside
the Introduction to Computing textbook by Dr. Yale Patt and Dr. Sanjay Patel.
With `lc3tools`, we strive to achieve the following goals:
* Create a cross-platform assembler and simulator.
* Develop three frontends: command-line, GUI, and grader.
* Separate the backend and frontend code so that the same backend can be used
  along with any frontend, leading to no behavior fragmentation.
* Design a powerful and easy-to-use grading framework.
* Write a maintainable, easily extendable, and open source code base.

Note on cross-platform compatibility: The two major problems with the existing
simulator are 1) that the code base has fragmented on Windows and Linux such that
behavior between the two platforms is inexplicably 2) macOS has no official support.
The goal of `lc3tools` to be truly cross platform implies that not only will complete
functionality be supported on all major platforms, but also that the code should
be compileable on these platforms.


# Building
`lc3tools` uses CMake for the build system and requires a C++11 compiler.
Building has been tested on Linux, macOS, and Windows (under MSYS). You
can build the project with the following commands
```
# create build directory
mkdir build && cd build
# set up build directory
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake ..
# build
make
```

Building the project will generate a `bin` directory inside the `build`
directory that contains the executables.

# Usage
Currently the following executables are produced upon building on Linux
(equivalent files created for macOS and Windows):
* `lib/liblc3core.so` is the backend.
* `bin/assembler` is a CLI frontend for the assembler.
* `bin/simulator` is a CLI frontend for the simulator.
* `bin/*` are executables for each grader

## assembler
`assembler` takes assembly files as inputs and generates object files with the
same name. For example, runnning `assembler proj1.asm` will generate the files
`proj1.obj`.

## simulator
`simulator` takes object files as inputs and executes them on an LC-3 simulator.
More detailed directions on how to use the simulator shell can be accessed by
typing in `help` after running the simulator.
