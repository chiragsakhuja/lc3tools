# lc3tools [![Build Status](https://travis-ci.org/chiragsakhuja/lc3tools.svg?branch=master)](https://travis-ci.org/chiragsakhuja/lc3tools)
`lc3tools` is a set of tools to build code for and simulator the LC-3 system described in *Introduction to Computing* by
Dr. Yale Patt and Dr. Sanjay Patel. The motivation behind creating a new set of tools is to address the following
pitfalls of the existing tools.
* Lack of consistent cross-platform and behavior.
* Lack of any available simulator on macOS, lack of GUI on Linux, and lack of command line tools on Windows.
* Missing source code.
* Ad-hoc grading system built on top of parsing output and without real support for interrupts.

`lc3tools` is open-source and requires only cross-platform (i.e. Windows, macOS, and Linux) development tools to build.
Additionally, the tools runs as standalone executable packages on all three major platforms. The executables include a
command line assembler, simulator, and graders as well as a GUI.  All executables interface with a common static library
so that behavior is consistent between tools on a platform and between platforms.

# Usage

## assembler
`assembler` accepts assembly or binary files as arguments and generates object files alongside the sources. The
operation is determined by the extension of the source file. A `.asm` extension indicates the assembler should be
invoked and a `.bin` extension indicates the binary converter should be invoked. Full operation of the `assembler`
command is as follows:
```
assembler [--print-level=N] [FILE ...]

  --print-level=N    (default=6) a number 0-9 to indicate verbosity of the output
  FILE               a source file to be built
```

## simulator
`simulator` accepts object files as arguments and loads them into the LC-3 system. The first object file in the argument
list will determine the initial PC. Once the object files are loaded, the simulator can be controlled by an interactive
prompt. Details on usage of the simulator can be found by typing `help` in the prompt. Full operation of the `simulator`
command is as follows:
```
simulator [--print-level=N] [FILE ...]

  --print-level=N    (default=6) a number 0-9 to indicate verbosity of the output
  FILE               an object file to be loaded
```

## Graders
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

# Building
Detailed instructions on how to build the executables can be found in BUILD.md.
