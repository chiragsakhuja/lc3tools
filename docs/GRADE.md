# Graders
Graders are standalone executables that consume LC-3 source code in binary
(`*.bin`) or assembly (`*.asm`), perform a series of tests, and output a report.
They are written in C++ and use the static library produced alongside the 
command line tools as well as a simple framework to interact with LC-3 programs
in real time as they run. Graders are also compiled alongside the command line
tools.

Graders are written in a similar fashion to unit tests. There will be a single
grader for each assignment that defines the test cases. In practice (i.e. in a classroom setting) there may be scripts that run each students' assignments 
through a grader executable and then aggregate the results.

Grader source files live in the `frontend/grader/labs/` directory. When they are
built, as per the [build document](BUILD.md), the executables will be in the
`build/bin/` directory with the same name as the source of the grader.

# Tutorial
This tutorial will cover all the steps necessary to create a grader for a simple
assignment. This tutorial will assume you are using a *NIX system (macOS or 
Linux), although Windows works fine. For a Windows system, adjust the build
commands as described in the [build document](BUILD.md#Windows).

## Assignment Description
Write an LC-3 assembly program that performs unsigned addition on a set of numbers in memory and saves the result in location 0x3100. The set of numbers
begins at location 0x3200 and continues until the value 0x0000 is encountered in
a memory location. You may ignore overflow and you may assume there will be no
more than 2048 total numbers to add. Your program must start at location 0x3000.

### Solution
The following assembly program accomplishes the task in the description above:

```
.orig x3000

; intialize registers
;   r0: accumulator
;   r1: address of next value to load
;   r2: temporary space to hold loaded value
        and r0, r0, #0
        ld r1, start

; load value and accumulate until 0 is found
loop    ldr r2, r1, #0
        brz done
        add r0, r0, r2
        add r1, r1, #1
        br loop

; store result and halt
done    sti r0, result
        halt

start   .fill x3200
result  .fill x3100

.end
```

You can create this file in the root directory as `tutorial_sol.asm`.

## Creating a New Grader
From the root directory, navigate to `frontend/grader/labs/` and create a file
for this grader called `tutorial_grader.cpp`. Each grader is expected to define three functions. For now just define empty functions. As the tutorial progresses, explanations for each function will be provided. Fill in the 
following code in `tutorial_grader.cpp`:

```
#include "../framework.h"

void testBringup(lc3::sim & sim) {}

void testTeardown(lc3::sim & sim) {}

void setup(void) {}
```

To build the grader, you must first rerun CMake to make it aware of the new
source file Then you may build the grader. Navigate to the `build/` directory
that was created during the [initial build](BUILD.md) and invoke the following 
commands:

```
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

If all goes well, compilation should succeed and you should see the file
`build/bin/tutorial_grader` produced. Once the grader has been built for
the first time, it suffices to just run the `make` command from the `build/`
directory to rebuild.