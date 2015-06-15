# lc3sim
`lc3sim` is a complete overhaul of the current LC-3 simulator used in EE 306 at
The University of Texas at Austin.  With `lc3sim`, we strive to achieve the
following goals:
* Create a truly cross platform assembler and simulator.
* Design an interface that unifies editing, assembling, and simulating code.
* Develop a formal testing infrastructure that can be used with equal ease by
  both students and graders.
* Integrate a submission system with the interface to encourage frequent
  submissions and/or version control.
* Maintain an organized, easily extendable, and open source code base.

Note on cross-platform compatibility: The two major problems with the existing
simulator are that the code base has fragmented on Windows and Linux such that
different functionality is supported, and OS X has no official support. The goal
of `lc3sim` to be truly cross platform implies that not only will complete
functionality be supported on all major platforms, but also that the code should
be compileable on these platforms.

# Building
`lc3sim` uses CMake for the build system and it relies on nothing other than Flex
and Bison. On a Linux or OS X system, you can execute the following commands to
compile the project.

```
# create build directory
mkdir build && cd build
# configure directory for cmake
cmake .. -DCMAKE_BUILD_TYPE=Release
# generate build system
cmake ..
# build all the targets
make all
```

Building the project will generate a `bin` directory inside the `build`
directory that contains the targets.

# Usage
Currently two targets are produced upon building:
* `lc3as` is the assembler.
* `liblc3utils.a` is a static library used internally.

## lc3as
`lc3as` takes assembly files as inputs and generates assembled files with the
same name. For example, runnning `lc3as proj1.asm` will generate the files
`proj1.bin`, `proj1.hex`, and `proj1.obj`.
