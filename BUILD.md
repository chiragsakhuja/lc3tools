# Building lc3tools

lc3tools uses CMake to build the command line tools and Yarn to build the GUI.

## Command line tools
The command line tools require a C++11 compiler and CMake to be installed.

### *NIX system
To build on macOS and Linux, you may invoke the following commands from the root of the repository:
```
# create build directory
mkdir build && cd build
# set up build directory
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=Release ..
# build
make
```

The assembler, simulator, and graders will be built under `build/bin`, and a static library will be built under
`build/lib`. The interface to the static library is described in depth in API.md.

### Windows
To build on Windows, Visual Studio, MSYS2, or some other build system can be used as long as it is supported by CMake.
If using Visual Studio, you may invoke `cmake-gui` to configure the project and then open it in Visual Studio. It is
necessary to build the Release configuration for the GUI.

The assembler, simulator, and graders will be built under `build/bin/Release`, and a static library will be built under
`build/lib/Release`.


## GUI
The GUI is built with Electron and requires Yarn and its dependencies to build. To run the GUI, invoke the following
commands from the `frontend/gui` directory.
```
# install packages
yarn
# build lc3 node module
yarn lc3
# run lc3tools
yarn run dev
```

Additionally, the GUI may be packaged into a standalone executable by invoking the following command:
```
yarn build
```

The resulting package will be in `frontend/gui/build` and its format will depend on the platform it is being built on.
