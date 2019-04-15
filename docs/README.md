# LC3Tools [![Build Status](https://travis-ci.org/chiragsakhuja/lc3tools.svg?branch=master)](https://travis-ci.org/chiragsakhuja/lc3tools)
LC3Tools is a modern set of tools to build code for and simulate the LC-3 
system described in *Introduction to Computing* by Dr. Yale Patt and Dr. Sanjay
Patel.

This project has the following aims:

* Consistent cross-platform support (across Windows, macOS, and Linux)
* Intuitive user interface
* Consistent and comprehensive support for a GUI and command line tools
* Powerful grading API for classroom settings
* Well-documented, simple, open-source code base

## Usage

All of the code for the project lives in this repository, but it can be broken
down into three logical categories: a set of command line tools, a GUI, and
a series of graders.

Building the command line tools and the graders only requires `CMake` and a
compiler that supports C++11. The command line tools and graders are built
simultaneously with a single command. More details on how to build these
categories can be found in the [BUILD document](docs/BUILD.md).

### assembler
`assembler` accepts assembly or binary files as arguments and generates object files alongside the sources. The
operation is determined by the extension of the source file. A `.asm` extension indicates the assembler should be
invoked and a `.bin` extension indicates the binary converter should be invoked. Full operation of the `assembler`
command is as follows:
```
assembler [--print-level=N] [FILE ...]

  --print-level=N    (default=6) a number 0-9 to indicate verbosity of the output
  FILE               a source file to be built
```

### simulator
`simulator` accepts object files as arguments and loads them into the LC-3 system. The first object file in the argument
list will determine the initial PC. Once the object files are loaded, the simulator can be controlled by an interactive
prompt. Details on usage of the simulator can be found by typing `help` in the prompt. Full operation of the `simulator`
command is as follows:
```
simulator [--print-level=N] [FILE ...]

  --print-level=N    (default=6) a number 0-9 to indicate verbosity of the output
  FILE               an object file to be loaded
```

### Graders
The grader for each lab is built as a separate executable. Each grader accepts assembly or binary files as arguments and
outputs the grade report for the program. Full operation of the grading executables is as follows:
```
grader [--print-output] [--print-level=N] [FILE ...]

  --print-output     show the console output along with the grade report (useful for debugging)
  --print-level=N    (default=1) a number 0-9 to indicate verbosity of the output
                     note: will affect the resulting grade but can be useful for debugging
  FILE               a source file to be built and graded
```

More details on how to write a grader can be found in GRADE.md.

## Building
Detailed instructions on how to build the executables can be found in BUILD.md.
