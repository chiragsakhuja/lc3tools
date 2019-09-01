# LC3Tools [![Build Status](https://travis-ci.org/chiragsakhuja/lc3tools.svg?branch=master)](https://travis-ci.org/chiragsakhuja/lc3tools)
LC3Tools is a modern set of tools to build code for and simulate the LC-3
system described in *Introduction to Computing* by Dr. Yale Patt and Dr. Sanjay
Patel.

This project has the following aims:

* Consistent cross-platform support (across Windows, macOS, and Linux)
* Consistent behavior across the GUI, command line tools, and other applications
* Intuitive user interface
* Powerful grading API for classroom settings
* Well-documented, simple, open-source code base

All of the code for the project lives in this repository, but it can be broken
down into three components: a set of command line tools, a GUI, and
a series of graders.

## Quick Start
Students trying to use LC3Tools can refer to the
[downloading/installing guide](DownloadingAndInstalling.pdf) and the
[beginniner's usage guide](GuideToUsingLC3Tools.pdf).

Instructors trying to use the grading infrastructure will need to [build from
source](README.md#building-from-source) and use the [command line
utilities](README.md#additional-information). The [grader document](GRADE.md)
will also be useful.

## Downloading Releases
Pre-built GUI packages for Windows, macOS, and Linux can be found in the
[Releases](https://github.com/chiragsakhuja/lc3tools/releases) tab. The
command line tools and graders must be built from source as described in
the following section.

## Building from Source
Building the command line tools and the graders only requires
[CMake](https://cmake.org) and a compiler that supports C++11. The command line
tools and graders are built simultaneously with a single command. More details
on how to build these two components can be found in the
[build document](BUILD.md#command-line-tools-and-graders).

The GUI can also be built from source but additionally requires
[NodeJS](https://nodejs.org/en/) and [Yarn](https://yarnpkg.com/en/) to be
installed. Directions on how to build the GUI can also be found in the
[build document](BUILD.md#gui).

## Additional Information
More details on how to use the individual components can be found in the
[command line tools](CLI.md), [GUI](GUI.md), and [graders](GRADE.md) documents.

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
