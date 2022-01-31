**Disclaimer:** This document reflects the testing framework for
release v2.0.0, which is incompatible with prior versions. To see details on the
previous testing framework, which is now deprecated, see [this
document](TEST1.md).  For help transitioning, see [this document](UPGRADE1.md).

# Table of Contents

* [Tutorial](TEST.md#tutorial)
  * [Assignment Description](TEST.md#assignment-description)
  * [Creating a Unit Test](TEST.md#creating-a-unit-test)
  * [Adding a Test Case](TEST.md#adding-a-test-case)
  * [Adding Another Test Case](TEST.md#adding-another-test-case)
  * [Conclusion](TEST.md#conclusion)
  * [Appendix: Common Paradigms](TEST.md#appendix-common-paradigms)
* [Additional Resources](TEST.md#additional-resources)

# Unit tests
Unit tests in this framework are standalone executables that consume LC-3 source
code in binary (`*.bin`) or assembly (`*.asm`), perform a series of tests, and
output a report. In the context of LC3Tools, unit testing will mostly be used by
instructors to automatically grade assignments, but students are encouraged to
use the framework and other advanced debugging features for their own
assignments as well. Unit tests are written in C++ and enable fine control and
access to the LC-3 system while a program is running. Unit tests are also
compiled alongside the command line tools.

This document is geared toward instructors who are writing their first unit
tests in a classroom setting. The bulk of the document is in the form of a
tutorial, and many more details are provided in the [additional
resources](TEST.md#additional-resources) section. Please read the [build
document](BUILD.md#command-line-tools-and-unit-tests) and the [command line
tools document](CLI.md) before reading this document.

Generally there will be a single unit test for each assignment. The unit test
will consist of a set of test cases. The unit test executable consumes a single
student's assignment and outputs a report for that student. In practice, the
instructor will need scripts that invoke the unit test for each student and then
aggregate the results from the reports. We provide such a script, as described
in the [Grader document](GRADER.md), that facilitates batch grading and even
interfaces with Canvas, the learning management system used at UT Austin, to
download student information and upload grades and unit test reports.

Unit test source files live in the `src/test/tests` directory. When a unit test
is built, as per the [build
document](BUILD.md#command-line-tools-and-unit-tests), the executable will be
generated in the `build/bin` directory on \*NIX systems and `build/Release/bin`
on Windows. The executable be named the same as the source file for the unit
test (e.g. a unit test labeled `assignment1.cpp` will produce an executable
called `assignment1`).

# Tutorial
This tutorial covers all the steps necessary to create a unit test for a simple
assignment. This tutorial assumes you are using a *NIX system (macOS or Linux),
although Windows also works. For a Windows system, adjust the build commands as
described in the [build document](BUILD.md#windows).

**It is recommended that you follow this tutorial from top to bottom to get a
better understanding of how to utilize the testing framework.**

## Assignment Description
Write an LC-3 assembly program that performs unsigned addition on a set of
numbers in memory and saves the result in location 0x3100. The set of numbers
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

Create this file in the root directory of LC3Tools as `tutorial_sol.asm`.

## Creating a Unit Test
From the root directory, navigate to `src/test/tests/` and create a file for
this unit test called `tutorial.cpp`. Each unit test is expected to define four
functions. For now just define empty functions. As the tutorial progresses,
explanations for each function will be provided. Fill in the following code in
`tutorial.cpp`:

```
#define API_VER 2
#include "framework.h"

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester) { }

void shutdown(void) { }
```

To build the unit test, you must first rerun CMake to make it aware of the new
unit test file. Then you may build the unit test. Navigate to the `build/`
directory that was created during the [initial
build](BUILD.md#command-line-tools-and-unit-tests) and invoke the following
commands:

```
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

You should see a new executable at `build/bin/tutorial`. Once the unit test has
been built for the first time, it suffices to just run the `make` command from
the `build/` directory to rebuild.

**Note that you must have the `#define API_VER 2` line to use the correct API
version.**

## Adding a Test Case
A test case takes the form of a function and takes three arguments of the
following types:

1. `lc3::sim &`: A reference to the simulator object that is running the input
   program.
2. `Tester &`: A reference to the testing framework.
3. `double`: The total number of points allocated for the test case.

Note that the simulator object is reinitialized between test cases to prevent
contamination.

For the first test case, check if the input program terminates correctly when
there are 0 numbers in the input (i.e. the value at location 0x3200 is 0).

First, define a new function:

```
void ZeroTest(lc3::sim & sim, Tester & tester, double total_points)
{
}
```

Since the simulator is reinitialized for every test case, it is always necessary
to initialize the PC as well as other input values. In this case that means
additionally initializing location 0x3200. Initialize the values by adding the
following code to the `ZeroTest` function:

```
sim.writePC(0x3000);
sim.writeMem(0x3200, 0);
```

The `writePC` function, as expected, sets the PC to location 0x3000. The
`writeMem` function sets the value at memory location 0x3200 (i.e. the first
argument) to 0 (i.e.  the second argument).

Next the test case should actually run the input program so it can verify the
results. It is usually best to restrict the total number of instructions that
are executed so that the unit test terminates even if the input program does not.
To be safe, set the instruction limit to 50000 instructions (which will execute
in well under 1 second) and then run the input program. Add the following lines
to the `ZeroTest` function.

```
sim.setRunInstLimit(50000);
sim.run();
```

The `setRunInstLimit` function sets the maximum number of instructions to 50000.
The `run` function will execute the input program until it halts or the
instruction limit is reached.

Finally, verify that the result is correct by adding the following line to the
`ZeroTest` function:

```
tester.verify("Is Zero?", sim.readMem(0x3100) == 0, total_points);
```

The `readMem` function returns the value at location 0x3100.

The `verify` function takes the following three arguments:

1. `std::string`: The message to print in the report for this particular check.
2. `bool`: The condition that must be satisfied to earn points.
3. `double`: The number of points allocated for this particular check.

A single test case may invoke the verify function any number of times and
allocate any amount of points to each check. In this case there is only a single
check, that the final value in memory location 0x3100, so assign the full number
of points to the check.

That's it! It only took 5 lines to create a simple test case. The last step is
to make sure the testing framework invokes the test case. To do this, add the
following line to the `setup` function.

```
tester.registerTest("Zero Test", ZeroTest, 10, false);
```

The `setup` function is called one time before any test cases are run. It can be
used to register the test cases as well as initialize any global variables that
the unit test needs to keep track of.

The `registerTest` function informs the testing framework that it should invoke
a test case and takes the following arguments:

1. `std::string`: The name of the test case.
2. `void(lc3::sim &, Tester &, double)`: A function pointer to test case.
3. `double`: The total number of points allocated for the test case.
4. `bool`: Whether to randomize the machine before running the test case
   (`true`) or not (`false`).

### Running the Unit Test
Build the unit test by running the `make` command from the `build/` directory.
To run the unit test, simply invoke the `tutorial` executable with
`tutorial_sol.asm` as an argument. This can be done by running the following
command from the root directory:

```
build/bin/tutorial tutorial_sol.asm
```

The output should be as follows:

```
attempting to assemble tutorial_sol.asm into tutorial_sol.obj
assembly successful
==========
Test: Zero Test
  Is Zero? => Pass (+10 pts)
Test points earned: 10/10 (100%)
==========
==========
Total points earned: 10/10 (100%)
```

## Adding Another Test Case
The following test case will test an actual array of numbers:

```
void SimpleTest(lc3::sim & sim, Tester & tester, double total_points)
{
    // Initialize PC and memory locations
    sim.writePC(0x3000);

    uint16_t values[] = {5, 4, 3, 2, 1, 0};
    uint64_t num_values = sizeof(values) / sizeof(uint16_t);
    uint16_t real_sum = 0;

    for(uint64_t i = 0; i < num_values; i += 1) {
        sim.writeMem(0x3200 + static_cast<uint16_t>(i), values[i]);
        real_sum += values[i];
    }

    // Run test case
    sim.setRunInstLimit(50000);
    sim.run();

    // Verify result
    tester.verify("Is Correct?", sim.readMem(0x3100) == real_sum, total_points);
}
```

Also, register the test case to be valued at 20 points by adding the following
line to the `setup` function.

```
tester.registerTest("Simple Test", SimpleTest, 20, false);
```

After rebuilding the unit test and running it, you should see the following output:

```
attempting to assemble tutorial_sol.asm into tutorial_sol.obj
assembly successful
==========
Test: Zero Test
  Is Zero? => Pass (+10 pts)
Test points earned: 10/10 (100%)
==========
Test: Simple Test
  Is Correct? => Pass (+20 pts)
Test points earned: 20/20 (100%)
==========
==========
Total points earned: 30/30 (100%)
```

### Refactoring with `testBringup` and `testTeardown`
You may note that setting the PC and the instruction limit are redundant for all
test cases. The `testBringup` and `testTeardown` functions can be used to remove
some redundancy. These functions are run before and after, respectively, each
test case. This is unlike the `setup` function which is run only once before any
test cases (before the first `testBringup`).

To remove some redundancy in the initialization of the test cases, add the
following lines to the `testBringup` function and remove them from the
`ZeroTest` and `SimpleTest` functions:

```
sim.writePC(0x3000);
sim.setRunInstLimit(50000);
```

As an aside, the `shutdown` function is called once after all the test cases
have run (after the last `testTeardown`) and can be used to clean up any global
variables that were initialized in the `setup` function for the unit test to
use.

## Conclusion
The full source code of this tutorial can be found in
[src/test/tests/samples/tutorial_grader.cpp](https://github.com/chiragsakhuja/lc3tools/blob/master/src/test/tests/samples/tutorial_grader.cpp).
This tutorial covered a small subset of the capabilities of the unit testing
framework and API. Some other features include: easy-to-use I/O checks;
callbacks before and after instruction execution, subroutine calls, interrupts,
etc.; and control over every element of the LC-3 state. Full details can be
found in the [API document](API.md).

## Appendix: Common Paradigms
Some common paradigms can be found across test cases, such as supplying input
and checking output. The descriptions of each of the functions in this section
can be found in the [API document](API.md).

### Successful Exit Paradigm
There are typically two conditions for a successful exit: the input program does
not trigger any LC-3 exceptions and it does not exceed the instruction limit.
The variants of the `run` functions, detailed in the [API document](API.md),
return a boolean based on the status of execution. If the return value is
`true`, the program did not trigger any exceptions. The `didExceedInstLimit`
function returns whether or not the program exceeded the instruction limit.
Assuming the limit is set to a reasonably high number, exceeding the limit
typically means the program did not halt.

Thus, the following simple check can be added at the end of each test case to
verify the program behaved correctly.

```
bool success = sim.runUntilHalt();
tester.verify("Correct Execution?", success && ! sim.didExceedInstLimit(), 0);
```

### I/O Paradigm (Polling)
Assume you would like to grade an assignment that prints a prompt, requests
input, does something with the input, then prints the prompt again. This process
repeats until the user types in a response that quits the program. For example,
take a program that repeats the inputted character 5 times:

```
Enter a character (q to exit): a
aaaaa
Enter a character (q to exit): b
bbbbb
Enter a character (q to exit): q
```

A test case could be written using the I/O API, detailed in the [API
document](API.md#automated-input).

```
bool success = true;
bool check;

success &= sim.runUntilInputRequested();
check = tester.checkMatch(tester.getOutput(), "Enter a character (q to exit): ");
tester.verify("Is Prompt Correct?", check, total_points / 4);

tester.clearOutput();
tester.setInputString("a");
success &= sim.runUntilInputRequested();
check = tester.checkContain(tester.getOutput(), "aaaaa");
tester.verify("Is 'a' Correct?", check, total_points / 4);

tester.clearOutput();
tester.setInputString("b");
success &= sim.runUntilInputRequested();
check = tester.checkContain(tester.getOutput(), "bbbbb");
tester.verify("Is 'b' Correct?", check, total_points / 4);

tester.setInputString("q");
success &= sim.runUntilHalt();
tester.verify("Correct execution?", success && ! sim.didExceedInstLimit(), total_points / 4);
```

The first set of lines verify that the prompt is correct before sending any
input.  `runUntilInputRequested` allows the entire prompt to print and then
pauses simulation as soon as any input is requested. Thus, the only output that
has been generated so far will be the prompt.

The next set of lines clears the output buffer, which erases the prompt and any
other output the simulation has created thus far. Then the input can be set.
Finally, the output is checked to see if it contains the duplicated input. This
sequence is checked for the input "a" and "b".

The final set of lines verifies that the program exits properly as described in
the [Successful Exit Paradigm](TEST.md#successful-exit-paradigm).

**Important Note about I/O**: Remember that the newline character is considered
input like any other key. As such, you must add a `\n` to the end of the string
provided to `setInputString` function if the program expects a newline character
as input.

### I/O Paradigm (Interrupt)
The I/O Paradigm for interrupt-driven input is similar to the paradigm for
polling, so please read [that section](TEST.md#io-paradigm-polling) before.

The main difference between interrupt-driven and polling-driven paradigms is
that `runUntilInputRequested` can no longer be reliably used since the program
will never request for input. Instead the program must run normally after the
input string has been set.

```
tester.setInputString("a");
tester.setInputCharDelay(50);
bool success = sim.run();
bool check = tester.checkContain(tester.getOutput(), "aaaaa");
tester.verify("Correct output", check, total_points / 2);
tester.verify("Correct execution", success && ! sim.didExceedInstLimit(), total_points / 2);
```

The `setInputCharDelay` delays the input from being sent until 50 instructions
have executed. This is useful in the context of interrupts because interrupts
are disabled by default and the program must execute a handful of instructions
to enable them. Note that `setInputCharDelay` applies to every character, not
the string as a whole, and remains set until it is changed again. This is, in
turn, useful because the interrupt service routine typically executes many
instructions. It is often beneficial, though not necessary, for the test case to
allow the entire ISR to execute before triggering another interrupt.

# Additional Resources

## API

The [API document](API.md) contains a comprehensive description of all of the
features that the simulator and testing framework provide.

## Sample Unit Tests and Solutions

Several sample unit tests are provided in the `src/test/tests/samples` directory
that can be used as reference. They are the unit tests that have been developed
for all of the real assignments over 2 semesters of the introductory ECE course
at UT Austin. While the samples are not comprehensive, the do cover a thorough
range of assignment types such as complex interactive programs (e.g.  `nim`),
interrupt-driven I/O (`interrupt1`), and even algorithms with strict time
complexity (`polyroot`). In particular, each sample exemplifies testing features
as follows:

1. `binsearch`: complex data initialization; I/O paradigm (polling); simulator
   randomization
2. `interrupt1`: exception checking; I/O paradigm (interrupt); simulator
   randomization
3. `interrupt2`: exception checking; I/O paradigm (interrupt); simulator
   randomization
4. `intersection`: complex data initialization; complex verification; exception
   checking; simulator randomization
5. `nim`: complex I/O interaction; complex verification; exception checking;
   fuzzy string matching; I/O paradigm (polling); simulator randomization;
   string preprocessing
6. `polyroot`: exception checking; simulator callbacks; simulator randomization;
   time complexity verification
7. `pow2`: exception checking; simulator randomization
8. `rotate`: detailed report messages; exception checking; simulator
   randomization
9. `shift`: detailed report messages; exception checking; simulator
   randomization
10. `sort`: detailed report messages; exception checking; simulator
    randomization

A more in-depth description of each assignment can be found in the [Sample
Assignments document](SampleAssignments.pdf).

Assembly/binary solutions som assignments  are also
provided in `src/test/tests/samples/solutions`. To verify the unit test's
functionality, you may run the following from the root directory after
[compiling the command line tools](BUILD.md#command-line-tools-and-unit-tests)
with samples enabled (default).

```
build/bin/binsearch src/test/tests/samples/solutions/binsearch.asm
build/bin/interrupt1 src/test/tests/samples/solutions/interrupt1.asm
build/bin/interrupt2 src/test/tests/samples/solutions/interrupt2.asm
build/bin/intersection NOT-PROVIDED
build/bin/nim NOT-PROVIDED
build/bin/polyroot src/test/tests/samples/solutions/polyroot.asm
build/bin/pow2 src/test/tests/samples/solutions/pow2.bin
build/bin/rotate src/test/tests/samples/solutions/rotate.bin
build/bin/shift src/test/tests/samples/solutions/shift.bin
build/bin/sort NOT-PROVIDED
```

Furthermore, `interupt1.asm` and `interrupt2.asm` show two different methods for
enabling interrupts. The former creates a new `TRAP` instruction that can be
called from the user program and the latter starts off with some code in system
space that enables interrupts and then jumps to to the user program with an
`RTI`. Either of these methods may be useful to provide to students as starter
code, since they require slightly more advanced knowledge of the LC-3 than some
classes may cover.

## Debugging the Program Under Test
In a classroom environment, it is very often the case that students will ask
an instructor for more details on their score or just to help with debugging.
The command line tooles and unit testing framework enable significantly more
debugging capability than the GUI, which is often what the student will be
using. More details on this extra functionality can be found in the [debugging
document](DEBUG.md).

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
