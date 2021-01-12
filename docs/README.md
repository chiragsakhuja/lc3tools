# LC3Tools [![Build Status](https://travis-ci.org/chiragsakhuja/lc3tools.svg?branch=master)](https://travis-ci.org/chiragsakhuja/lc3tools)
LC3Tools is a modern set of tools to build code for and simulate the LC-3
system described in *Introduction to Computing* by Dr. Yale Patt and Dr. Sanjay
Patel.

This project has the following aims:

* Consistent cross-platform support (across Windows, macOS, and Linux)
* Consistent behavior across the GUI, command line tools, and other applications
* Intuitive user interface
* Powerful testing API for unit tests and auto-graders
* Well-documented, simple, open-source code base

All of the code for the project lives in this repository, but it can be broken
down into three components: a set of command line tools, a GUI, and a series of
unit tests.

## Quick Start
LC3Tools has two primary use cases.  Students will genenerally use the GUI,
which can be downloaded from the
[Releases](https://github.com/chiragsakhuja/lc3tools/releases) tab.  For
information on which download is correct for you, refer to the
[downloading/installing guide](DownloadingAndInstalling.pdf).  To learn how to
use the GUI, refer to the [beginniner's usage guide](GuideToUsingLC3Tools.pdf).

Instructors, or students who are comfortable with the command line, will
generally use the command line tools, which is the only way to write unit tests.
Note that auto-graders are written as unit tests. To get started, follow these
steps, which can be performed on any supported platform:
1. Ensure you have [CMake](https://cmake.org) and a C++11 compiler installed.
2. Build the command line utilities from source as described in the 
   [build document](BUILD.md#command-line-tools-and-unit-tests).
3. Follow the unit test tutorial in the [unit test document](TEST.md).
4. Refer to the [command line tools document](CLI.md).
5. Refer to the samples provided in the `src/test/tests/samples` directory.
6. Refer to the [API document](API.md) to learn about the full unit testing
   capabilities.

In some cases, instructors may want to modify the GUI as well.  In addition to
the requirements for the command line utilities, you will need to have
[NodeJS](https://nodejs.org/en/) and [Yarn](https://yarnpkg.com/en/) installed.
The [build document](BUILD.md#gui) also contains directions on how to build the
GUI.

## Additional Information
More details on how to use the individual components can be found in the
[command line tools](CLI.md), [GUI](GUI.md), and [unit test](TEST.md)
documents.

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
