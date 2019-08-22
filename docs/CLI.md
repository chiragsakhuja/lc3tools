# Command Line Tools
The command line tools include an `assembler` executable, a `simulator`
executable, and a static library that is used by both tools as well as the GUI
and graders. The graders are also interfaced through the command line, and more
information about them can be found in the [grading document](GRADE.md).

Assuming you have followed the [build document](BUILD.md), the `assembler` and
`simulator` executables will be under `build/bin/` on \*NIX systems and
`build/bin/Release/`on Windows systems. The static library will be under
`build/lib/` on \*NIX systems and `build/lib/Release/` on Windows systems.

## Assembler
The `assembler` executable accepts one or more assembly or binary files as
arguments and generates machine code files that the `simulator` executable can
use. If the filename has a `.asm` extension, the assembler will treat the input
file as an assembly program. If the filename has a `.bin` extension, the
"assembler" will treat the input file as a binary program (technically this is
just a conversion and not actually an "assembly"). Regardless of the input file
format, the `assembler` executable will produce an object file (extension
`.obj`) with the same name and in the same directory as the input file.

Full operation of the `assembler` executable is as follows:

```
assembler [--print-level=N] FILE [FILE ...]
  --print-level=N    (default=6) A number 0-9 to indicate the output verbosity
  FILE               A source file to be assembled or converted
```

## Simulator
The `simulator` executable accepts one or more object files (extension `.obj`)
and loads them into an emulated LC-3 system. The first object file in the
argument list will determine the initial PC, although it is trivial to change
the PC within the simulator itself.

Once the object files are loaded, the simulator is controlled with an
interactive shell-like interface. Details on the interface can be found by
typing in `help` in the simulator prompt. Full operation of the `simulator`
executable is as follows:

```
simulator [--print-level=N] FILE [FILE ...]
  --print-level=N    (default=6) A number 0-9 to indicate the output verbosity
  FILE               An object file to be loaded into the emulated LC-3 system
```

## Static Library
The static library is not directly accessible through the command line but is
built alongside the command line tools. The name of the static library depends
on the platform it is compiled for, but will be similar to `liblc3tools.a`.
This static library is referred to as "the backend" for this project.

The command line tools, [graders](GRADE.md), and [GUI](GUI.md) use the same
static library to perform all tasks, thus guaranteeing consistent behavior.
These components are all referred to as "frontends" for this project. Other
frontends can also be created by utilizing the API provided by the static
library. For more details on the API, see the [API document](API.md).

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
