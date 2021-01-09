# Table of Contents

* [Command Line Tools and Unit Test](BUILD.md#command-line-tools-and-unit-tests)
* [GUI](BUILD.md#gui-line-tools-and-unit-tests)

# Building from Source

There are two build systems in place for LC3Tools. The command line tools and
unit tests are built simultaneously and only require [CMake](https://cmake.org)
and a C++11 compiler. The GUI additionally requires
[NodeJS](https://nodejs.org/en/) and [Yarn](https://yarnpkg.com/en/). Everything
can be built on any of the supported operating systems.

The command line tools and unit tests can be built independently of the GUI.
However, the GUI requires the command line tools and unit tests to be built
first.

## Command Line Tools and Unit Tests
Building the command line tools and unit tests require
[CMake](https://cmake.org) and a C++11 compiler to be installed. Building
instructions vary between [*NIX systems](BUILD.md#nix-system) (macOS and Linux)
and [Windows](BUILD.md#windows).


### Products of Building
The build process will produce a static library and several executables. At
the least, the products will be the following:

* `bin/assembler`: The command line tool for converting assembly programs and
  binary code into machine code files that can be consumed by the simulator
  (in custom `*.obj` format).
* `bin/simulator`: The command line tool for simulating LC-3 programs.
* `lib/liblc3core.a` (or some other platform-specific name): The
  static library that provides consistent behavior for each of the components.

Additionally, an executable for each unit test will also be produced with the
same name as the source file for the unit test. More details on the unit tests
can be found in the [testing framework document](GRADE.md).

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
unit tests will be built under `build/bin/`, and the static library will be
built under `build/lib/`.

The default build options also build several sample unit tests under
`build/bin`. To disable these unit tests from building, add the
`-DBUILD_SAMPLES=OFF` argument to the `cmake` commands.

### Windows
Building on Windows may be done with any build system that CMake supports (e.g.
Visual Studio, MSYS2, etc.). This document will focus on building with Visual
Studio 2019 Community Edition (free). Some of the build system files may need
to be changed for other build systems or earlier versions of Visual Studio.

Visual Studio 2019 has CMake built in, so building is straightforward.  Simply
open Visual Studio and select the `Open a Local Folder` option.  Open the
`lc3tools` directory and then set the build configuration to `Release`.  Build
using the keyboard shortcut `Ctrl+Shift+B`.

## GUI
**Ensure that you have already completed the steps outlined in the
[previous section](BUILD.md#command-line-tools-and-unit-tests).**

The GUI is built on the [Electron](https://electronjs.org/) framework and thus
requires [NodeJS](https://nodejs.org/en/) and [Yarn](https://yarnpkg.com/en/) to
be installed. Once the tools are installed, you may invoke the following
commands from the root directory (on all platforms):

```
# Navigate to GUI directory
cd src/gui
# Install packages
yarn
# Build LC3Tools NodeJS module
yarn lc3
```

### Running the GUI
To run the GUI in development mode, invoke the following command from the
`src/gui` directory:

```
yarn run dev
```

Development mode runs slowly, so once development is complete, invoke the
following command from the `src/gui` directory to build an optimized and
standalone executable:

```
yarn build
```

The standalone executable will be produced in `src/gui/build/` and its
format will depend on the operating system it is being built on (`.exe`,
`.app.`, etc.).

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.