# Upgrading unit tests and frontends from API_VER 1 to 2
Release v2.0.0 of LC3Tools introduces a similar, but new, API version that is no
longer backward compatible with version 1 of the API.  Since version 1 is now
deprecated, this guide helps you transition to version 2.  The biggest changes
in the documented API have just been in some renamed functions, but behavior is
mostly the same, so the transition shouldn't be too difficult.

## Who is this for?
I'm gearing this guide toward instructors who intend on carrying over old
graders (now referred to as unit tests) to future semesters.  For example, this
may be because you are using the same assignment between semesters.  LC3Tools
does provide a compatibility layer for API_VER 1, which supports the
[documented API](API1.md), but API_VER 2 is the recommended API now. Also, note
that undocumented code may have changed and is not supported by the
compatibility layer.  **This document assumes you've completely read the
[updated unit test tutorial](TEST.md).**

This change may also impact anyone who has developed their own frontend.
Release v2.0.0 introduces a completely new simulator backend as well as some
minor bugfixes that are not in v1.0.6.  The new backend is functionality
identical to the previous backend, but it more stable and is fully
deterministic.  If these changes are important to you, please transition to
`API_VER 2`.

## New Directories
The `backend` and `frontend` categorization no longer applies, and everything is
in a single `src` directory now.  Unit tests (formerly referred to as graders)
have moved from `frontend/grader/labs` to `src/test/tests`.  Otherwise, behavior
is the same---you run CMake to detect new unit test sources files that are built
alongside the other command line utilities.

## API_VER Define
**To use API_VER 2, you must define it in your `.cpp` file before including any
LC3Tools header files. Add the following line to the top of your unit test.**

```
#define API_VER 2
#include "framework.h"
...
```

## API Changes
* [Renamed Simulator Functions](UPGRADE1.md#renamed-simulator-functions)
* [Callbacks](UPGRADE1.md#callbacks)
* [Unit Test Framework](UPGRADE1.md#unit-test-framework)
* [Automated Input](UPGRADE1.md#automated-input)
* [Output Checking](UPGRADE1.md#output-checking)
* [Verification Checks](UPGRADE1.md#verification-checks)
* [New Functionality](UPGRADE1.md#new-functionality)


### Renamed Simulator Functions
The functions that used to interact with the `lc3::sim` object have been
renamed.  Functionality is identical.  This table shows the names in API_VER 1
and 2.

| API_VER 1         | API_VER 2              |
| ----------------- | ---------------------- |
| getReg            | readReg                |
| setReg            | writeReg               |
| getMem            | readMem                |
| setMem            | writeMem               |
| getPC             | readPC                 |
| setPC             | writePC                |
| getPSR            | readPSR                |
| setPSR            | writePSR               |
| getMCR            | readMCR                |
| setMCR            | writeMCR               |
| getCC             | readCC                 |
| setCC             | writeCC                |
| setMemString      | writeStringMem         |
| runUntilInputPoll | runUntilInputRequested |

### Callbacks
In API_VER 2, the `callback_func_t` type is now a function pointer that takes
two arguments.  The first is of type `lc3::core::CallbackType`, which specifies
the type of callback being triggered, and the second is no longer
`lc3::core::MachineState &`, which was undocumented, but rather just a reference
to the simulator object, `lc3::sim &`, which is documented.  Furthermore, there
is a single function to register callbacks, and the first argument indicates the
type of callback, specified as type `lc3::core::CallbackType`, that is being
registered.  See [this section of API document](API.md#callbacks) and the
`polyroot` sample at `src/test/tests/samples` for details on usage.

### Unit Test Framework
Unit tests are still made up of the four functions `setup`, `shutdown`,
`testBringup` and `testShutdown`.  Other than the `setup` function, the other
functions remain the same.  The `setup` function now accepts a single `Tester &`
object that is used to register tests.  Rather than the `REGISTER_TEST` and
`REGISTER_RANDOM_TEST` macros, now use the `registerTest` function specified
[here in the API document](API.md#tester).

The arguments provided to each test case have changed.  Previously, each test
case was provided a reference to an `lc3::sim` object and a reference to an
`StringInputter` object.  Now each unit test is provided with a reference
to an `lc3::sim` object, a reference to a `Tester` object, and the total number
of points allocated to the test case.  All prior functionality, such as I/O
manipulation, verification, and scoring, is provided through these objects.

### Automated Input
The `StringInputter` object is no longer passed into each test case.  Instead,
input is handled through the `Tester &` object using the functions
`setInputString` and `setInputCharDelay`.  Also, as described in the [Renamed
Simulator Functions](UPGRADE1.md#renamed-simulator-functions) section, the
`runUntilInputPoll` function has been renamed to `runUntilInputRequested` and is
unchanged in behavior.  See [this section of the unit test
tutorial](TEST.md#appendix-common-paradigms), [this section of the API
document](API.md#automated-input), and the samples at `src/test/tests/samples`
for details on usage.

### Output Checking
Output is now accessible to each test case through the `Tester::getOutput`
function rather than hidden behind the `VERIFY_OUTPUT`-style macros.  The API
provides some new string manipulation functions, detailed in [this section of
the API document](API.md#string-manipulation-and-comparison), as well as an
explicit `clearOutput` function, that provide all the functionality, and more,
of API_VER 1.  See the samples at `src/test/tests/samples` for details on usage.

### Verification Checks
Verification is no longer performed by the `VERIFY`-style macros, which obscured
functionality and made strict assumptions about point distributions.  Instead,
the provided `Tester` object has a single `verify` function, documented in [this
section of the API document](API.md#tester).  The `verify` function is provided
with a condition that should be satisfied and the number of points that are
added if the condition is true.  Additionally, the `verify` function is provided
a message to be outputted in the report.  See the samples at
`src/test/tests/samples` for details on usage.

### New Functionality
The following new functionality has been added in `API_VER 2`.

*  A new set of string manipulation functions, such as fuzzy matching, that help
     facilitate output verification.
* An `output` function that allows the instructor to add useful messages to the
    report (when the `--tester-verbose` flag is passed at the command line).
* An `error` function that allows the instructor to explicitly report an error
    in grading.
* A couple of of new callback types, such as `INPUT_POLL`.
* `setBreakpoint` and `removeBreakpoint` functions that will pause execution as
    soon as the PC reaches a breakpointed address.
* A function to access the symbol table generated during assembly.  This pairs
    well with breakpoints, for example.

Details on all of these new functions are provided in the [API
document](API.md), and some example usage can be found in the samples at
`src/test/tests/samples`.

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
