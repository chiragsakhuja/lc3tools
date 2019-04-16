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

The tutorial below will help you get bootstrapped in writing graders.
Details on the grading framework and full API can be found in the
[API document](API.md).

# Tutorial
This tutorial will cover all the steps necessary to create a grader for a simple
assignment. This tutorial will assume you are using a *NIX system (macOS or
Linux), although Windows works fine. For a Windows system, adjust the build
commands as described in the [build document](BUILD.md#Windows).

It is recommended that you follow this tutorial from top to bottom to get a
better understanding of how to utilize the grading framework.

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
for this grader called `tutorial_grader.cpp`. Each grader is expected to define four functions. For now just define empty functions. As the tutorial progresses, explanations for each function will be provided. Fill in the
following code in `tutorial_grader.cpp`:

```
#include "../framework.h"

void testBringup(lc3::sim & sim) {}

void testTeardown(lc3::sim & sim) {}

void setup(void) {}

void shutdown(void) {}
```

To build the grader, you must first rerun CMake to make it aware of the new
source file. Then you may build the grader. Navigate to the `build/` directory
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

## Adding a Test Case
A test case takes the form of a function. Test cases should accept a single
parameter of type `lc3::sim &`. Each test case is executed with a newly
initialized copy of the simulator to prevent contamination.

For the first test case, check if the program terminates correctly when there
are 0 numbers in the input (i.e. the value at location 0x3200 is 0).

First, define a new function:

```
void ZeroTest(lc3::sim & sim)
{
}
```

Since the simulator is reinitialized for every test case, it is always necessary
to initialize the PC as well as other input values. In this case that means
additionally initializing location 0x3200. Initialize the values by adding the
following code to the `ZeroTest` function:

```
sim.setPC(0x3000);
sim.setMem(0x3200, 0);
```

The `setPC` function, as expected, sets the PC to location 0x3000. The `setMem`
function sets the location 0x3200 (i.e. the first argument) to the value 0 (i.e.
the second argument).

Now all that is necessary is to run the input program and verify the result. It
is usually best to restrict the total number of instructions that are executed
so that the grader terminates even if the input program does not. To be safe,
set a limit of 50000 instructions (which will execute in well under 1 second) and then run the program by adding the following lines to the `ZeroTest` function:

```
sim.setRunInstLimit(50000);
sim.run();
```

The `setRunInstLimit` function sets the maximum number of instructions to 50000.
The `run` function will execute the input program until the program halts or the
instruction limit is reached.


Finally, verify that the result is correct by adding the following line to the
`ZeroTest` function:

```
VERIFY(sim.getMem(0x3100) == 0);
```

The `VERIFY` macro is part of the grading framework and is used to determine
how many points a test case will earn if correct. More information on how
points are assigned can be found in the
[API document](API.md#grading-framework).

That's it! It only took 5 lines to create a simple test case. As a final step,
the test case must be registered with the grading framework and assigned a
certain number of points. To do so, add the following line to the `setup`
function:

```
REGISTER_TEST(Zero, ZeroTest, 10);
```

The `setup` function is called one time before any test cases are run. It can
be used to register the test cases as well as initialize any variables that the
grader may keep track of.

The `REGISTER_TEST` function informs the grading framework that it should
run the `ZeroTest` function (i.e. the second argument) as a test case. The
grading framework will label the test as `Zero` (i.e. the first argument) in
the ouputted report. Finally, the test will be worth 10 points (i.e. the third
argument) total.

### Running the Grader
Build the grader by running the `make` command from the `build/` directory. To
run the grader, simply invoke the `tutorial_grader` executable with
`tutorial_sol.asm` as an argument. This can be done by running the following
command from the root directory:

```
build/bin/tutorial_grader tutorial_sol.asm
```

The output should be as follows:

```
Test: Zero
  sim.getMem(0x3100) == 0 => yes
Test points earned: 10/10 (100%)
==========
==========
Total points earned: 10/10 (100%)
```

Full operation of the grader executable is as follows:

```
grader [--print-level=N] FILE [FILE ...]
  --print-level=N    (default=6) A number 0-9 to indicate the output verbosity
  FILE               A source file to be assembled or converted
```

## Adding Another Test Case
The following test case will test an actual array of numbers:

```
void SimpleTest(lc3::sim & sim)
{
    // Initialize PC and memory locations
    sim.setPC(0x3000);

    uint16_t values[] = {5, 4, 3, 2, 1, 0};
    uint64_t num_values = sizeof(values) / sizeof(uint16_t);
    uint16_t real_sum = 0;

    for(uint64_t i = 0; i < num_values; i += 1) {
        sim.setMem(0x3200 + static_cast<uint16_t>(i), values[i]);
        real_sum += values[i];
    }

    // Run test case
    sim.setRunInstLimit(50000);
    sim.run();

    // Verify result
    VERIFY(sim.getMem(0x3100) == real_sum);
}
```

Also, register the test case to be valued at 20 points by adding the following
line to the `setup` function.

```
REGISTER_TEST(Simple, SimpleTest, 20);
```

After rebuilding the grader and running it, you should see the following output:

```
Test: Zero
  sim.getMem(0x3100) == 0 => yes
Test points earned: 10/10 (100%)
==========
Test: Simple
  sim.getMem(0x3100) == real_sum => yes
Test points earned: 20/20 (100%)
==========
==========
Total points earned: 30/30 (100%)
```

### Refactoring with `testBringup` and `testTeardown`
You may note that setting the PC and the instruction limit are redundant for all
test cases. The `testBringup` and `testTeardown` functions can be used to remove
some redundancy. These functions are run before and after, respectively, every
test case. This is unlike the `setup` function which is run only once before
any test cases (before the first `testBringup`).

To remove some redundancy in the initialization of the test cases, add the
following lines to the `testBringup` function and remove them from the
`ZeroTest` and `SimpleTest` functions:

```
sim.setPC(0x3000);
sim.setRunInstLimit(50000);
```

As an aside, the `shutdown` function is called once after all the test cases
have run (after the last `testTeardown`) and can be used to clean up any
variables that were initialized in the `setup` function for the grader to use.

## Conclusion
The full source code of this tutorial can be found in
[frontend/grader/labs/tutorial_grader.cpp](https://github.com/chiragsakhuja/lc3tools/blob/master/frontend/grader/labs/tutorial_grader.cpp).
This tutorial covered a small subset of the capabilities of the grading
framework and API. Some other features include: easy-to-use I/O checks; hooks
before and after instruction execution, subroutine calls, interrupts, etc.; and
control over every element of the LC-3 state. Full details can be found in the
[API document](API.md).