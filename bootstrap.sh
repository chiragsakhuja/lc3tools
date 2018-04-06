#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 2

echo "Grader executable is in build/bin"
echo "Example: ./s18_lab3 barak3.asm"
echo "If moving the executable, also move lc3os.obj with it"
