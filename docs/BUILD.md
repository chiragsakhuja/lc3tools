# Building from Source

There are two build systems in place for LC3Tools. The command line tools and
graders are built simultaneously and only require [CMake](https://cmake.org) and
a C++11 compiler. The GUI additionally requires
[NodeJS](https://nodejs.org/en/) and [Yarn](https://yarnpkg.com/en/).

The command line tools and graders can be built independently of the GUI.
However, the GUI requires the command line tools and graders to be built first.

## Command Line Tools and Graders
Building the command line tools and graders require [CMake](https://cmake.org)
and a C++11 compiler to be installed. Building instructions vary between
[*NIX systems](BUILD.md#nix-system) (macOS and Linux) and
[Windows](BUILD.md#windows).


### Products of Building
The build process will produce a static library and several executables. At
the least, the products will be the following:

* `assembler`: The command line tool for converting assembly programs and
  binary code into machine code files that can be consumed by the simulator
  (in custom `*.obj` format).
* `simulator`: The command line tool for simulating LC-3 programs.
* `liblc3tools.a` (or some other platform-specific name): The
  static library that provides consistent behavior for each of the components.

Additionally, an executable for each grader will also be produced with the same
name as the source file for the grader. More details on the grading executables
can be found in the [grading document](GRADE.md).

### *NIX system
To build on macOS and Linux, you may invoke the following commands from the
root directory:

```
# Create build directory
mkdir build && cd build
# Set up build directory (run twice)
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=Release ..
# Build
make
```

Relative to the root directory, the binaries for the command line tools and
graders will be built under `build/bin/`, and the static library will be built
under `build/lib/`.

### Windows
Building on Windows may be done with any build system that CMake supports (e.g.
Visual Studio, MSYS2, etc.). This document will focus on building with Visual
Studio. Some of the build system files may need to be changed for other build
systems on Windows, as they are untested.

To build on Windows, first create a `build/` directory in the root directory of
this project. Then create a Visual Studio project with the CMake GUI as
follows: Open the CMake GUI through the Start Menu or the command line via the
`cmake-gui` command. Select the root directory of the project as the source and
the `build/` directory as the target in the CMake GUI. Press the Configure
button and then the Generate button.

Open the Visual Studio solution produced in the `build/` directory. In Visual
Studio select the x64 Release configuration and build the solution.

Relative to the root directory, the binaries for the command line tools and
graders will be built under `build/bin/Release/`, and the static library will
be built under `build/lib/Release/`.

## GUI
**Ensure that you have already completed the steps outlined in the
[previous section](BUILD.md#command-line-tools-and-graders).**

The GUI is built on the [Electron](https://electronjs.org/) framework and thus
requires [NodeJS](https://nodejs.org/en/) and [Yarn](https://yarnpkg.com/en/) to
be installed. Once the tools are installed, you may invoke the following
commands from the root directory (on all platforms):

```
# Navigate to GUI directory
cd frontend/gui
# Install packages
yarn
# Build LC3Tools node module
yarn lc3
```

### Running the GUI
Electron applications require an additional step to create a standalone
executable. However, this step takes a couple of minutes to run, so it is
also possible to run the application through Yarn.

To run the application in development mode through Yarn, you may invoke the
following command from the `frontend/gui/` directory:

```
yarn run dev
```

To optionally create a standalone executable, you may invoke the following
command from the `frontend/gui/` directory:

```
yarn build
```

The standalone executable will be produced in `frontend/gui/build/` and its
format will depend on the platform it is being built on (`.exe`, `.app.`, etc.).

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.