**Disclaimer:** This document reflects the API for release v2.0.0, which is
incompatible with prior versions. To see details on the previous API, see [this
document](API1.md). For help transitioning, see [this document](UPGRADE1.md).

# Table of Contents

* [Running the Machine](API.md#running-the-machine)
* [Getting/Setting Machine State](API.md#gettingsetting-machine-state)
* [Callbacks](API.md#callbacks)
* [Miscellaneous](API.md#miscellaneous)
* [Test Cases](API.md#test-cases)
* [Automated Input](API.md#automated-input)
* [String Manipulation and Comparison](API.md#string-manipulation-and-comparison)

# Testing Framework API
The purpose of this document is to describe the subset of the LC3Tools API that
is relevant to unit testing. The full API can be found in
`src/backend/interface.h`.  It is highly recommended to read the [testing
framework document](TEST.md) before reading this document.

There are two main types to use to the API: [`lc3::sim`](API.md#lc3sim),
and [`Tester`](API.md#tester).

# `lc3::sim`
This is the main interface to the simulator and is how common operations, such
as reading and writing memory, are done.  The active `lc3::sim` object is
provided as an argument into each test case.

## Running the Machine

### `bool run(void)`
Run the machine from the location of the PC until completion. Completion is
indicated by setting the clock enable bit in the Machine Control Register (MCR),
typically as the last instruction of the HALT trap routine.

Return Value:

* `true` if program halted without any exceptions, `false` otherwise.

### `bool runUntilHalt(void)`
Run the machine from the location of the PC until a HALT (TRAP x25) instruction
is encountered. The HALT trap routine will not run. This is useful when
verifying register contents, as the HALT trap routine may clobber values.

Return Value:

* `true` if program halted without any exceptions, `false` otherwise.

### `bool runUntilInputRequested(void)`
Run the machine from the location of the PC until any keyboard input is
requested. This is useful when verifying I/O.  See the [I/O
Paradigm (Polling)](TEST.md#io-paradigm-polling) for more details on usage.

Return Value:

* `true` if program halted without any exceptions, `false` otherwise.

### `void setRunInstLimit(uint64_t inst_limit)`
Sets the instruction count limit. Regardless of the type of run, simulation will
halt as soon as the instruction limit is reached. This can ensure that the unit
test does not run indefinitely even if the program it's running does not halt.
The instruction limit pertains to the each successive `run*` function executed.

Arguments:

* `inst_limit`: The number of instructions to execute before halting simulation.

### `void setBreakpoint(uint16_t addr)`
Set a breakpoint, by address, that will pause execution whenever the PC reaches
it.

Arguments:

* `addr`: Address to place breakpoint on.

### `bool removeBreakpoint(uint16_t addr)`
Remove a breakpoint by address.

Arguments:

* `addr`: Address to remove breakpoint from.

## Getting/Setting Machine State

### `uint16_t readReg(uint16_t id) const`
Get the value of a register.

Arguments:

* `id`: ID of register to read from.

Return Value:

* Value in the register.

### `void writeReg(uint16_t id, uint16_t value)`
Set a register to a value.

Arguments:

* `id`: ID of register to write to.
* `value`: New register value.

### `uint16_t readMem(uint16_t addr) const`
Get the value of a memory location.

Arguments:

* `addr`: Memory address to read from.

Return Value:

* Value in the memory location.

### `void writeMem(uint16_t id, uint16_t value)`
Set a memory location to a value.

Arguments:

* `id`: Memory address to write to.
* `value`: New memory location value.

### `uint16_t readPC(void) const`
Get the value of the Program Counter (PC).

Return Value:

* Value of the PC.

### `void writePC(uint16_t value)`
Set the value of the Program Counter (PC).

Arguments:

* `value`: New PC value.

### `uint16_t readPSR(void) const`
Get the value of the Processor Status Register (PSR).

Return Value:

* Value of the PSR.

### `void writePSR(uint16_t value)`
Set the value of the Processor Status Register (PSR).

Arguments:

* `value`: New PSR value.

### `uint16_t readMCR(void) const`
Get the value of the Memory Control Register (MCR).

Return Value:

* Value of the MCR.

### `void writeMCR(uint16_t value)`
Set the value of the Memory Control Register (MCR).

Arguments:

* `value`: New MCR value.

### `char readCC(void) const`
Get the value of the Condition Codes (CC) as a character.

Return Value:

* Value of the CC as 'N', 'Z', or 'P'.

### `void writeCC(uint16_t value)`
Set the value of the Condition Codes (CC) as a character.

Arguments:

* `value`: New MCR value as 'N', 'Z', or 'P'.

### `void writeStringMem(uint16_t addr, std::string const & value)`
Set a series of memory locations to a null-terminated string.

Arguments:

* `addr`: Starting address of string.
* `value`: New value of memory locations.

## Callbacks
There are several hooks available that may be useful during testing
such as when counting the number of times a specific subroutine is called. All
callback functions must match the signature defined by `callback_func_t`, and
the available callbacks are defined by the `lc3::core::CallbackType` enum.

### `callback_func_t`
The callback function prototype that all callback functions must follow, defined
as `std::function<void(lc3::core::CallbackType type, lc3::sim & sim)>`.

Arguments:

* `type`: Hook trigger given by the `lc3::core::CallbackType` enum.
* `sim`: Reference to simulator.

### `lc3::core::CallbackType`
An enum that defines available hook triggers, described as follows and
enumerated in the order in which they are resolved if multiple are
pending.

* `PRE_INST`: Before the next instruction executes.
* `INT_ENTER`: Upon entering an interrupt service routine.
* `EX_ENTER`: Upon entering an exception handler.
* `EX_EXIT`: Upon exiting an exception handler using `RTI`.
* `INT_EXIT`: Upon exiting an interrupt service routine using `RTI`.
* `SUB_ENTER`: Upon entering a subroutine using `JSR`.
* `SUB_EXIT`: Upon exiting a subroutine using `RET`.
* `INPUT_REQUEST`: When input is requested and no characters are pending in the
*  buffer.
* `INPUT_POLL`: Any time input is requested.
* `POST_INST`: After the instruction finishes executing.

### `void registerCallback(CallbackType type, callback_func_t func)`
Register a function that is called when the hook given by `type` is triggered.

Arguments:

* `type`: Hook trigger given by the `lc3::core::CallbackType` enum.
* `func`: Function to call when the hook is triggered.

## Miscellaneous

### `bool didExceedInstLimit(void) const`
Check if instruction limit was exceeded. This is useful for verifying that a
program halted properly.

Return Value:

* `true` if the instruction limit was exceeded, `false` otherwise.

# `Tester`
Additionally, the testing framework, which is accessed by through
the `Tester` object, provides important functions for each
unit test and test case.

## Test Cases

### `test_func_t`
The function prototype that all test cases must follow, defined as
`std::function<void(lc3::sim & sim, Tester & tester, double total_points)>`

Arguments:

* `sim`: Interface to simulator.
* `tester`: Interface to testing framework.
* `total_points`: Total number of points allocated to this test case.

### `void registerTest(std::string const & name, test_func_t test_func, double points, bool randomize)`
Register a test case with the testing framework.

Arguments:

* `name`: Name of test case.
* `test_func`: Function to call when invoking test case.
* `points`: Number of points allocated to test case.
* `randomize`: `true` if machine should be randomized before running test case,
   `false` otherwise.

### `void verify(std::string const & label, bool pred, double points)`
If the condition specified by `pred` is `true`, increment the test case score by
`points` points to the test case. Otherwise, do not increment the test case 
score. Can be invoked any number of times by a test case. `points` can be
specified as any number and does not automatically max out to `total_points`
(see `test_func_t`), which can be useful if allowing bonus points.

Arguments:

* `label`: String to be shown in score report to identify this check.
* `pred`: Condition that must be true to earn points.
* `points`: Number of points to add to test case score.

### `void output(std::string const & message)`
Outputs a debugging message in the report if the `tester-verbose` command line
flag is provided. Useful when debugging test cases or when wanting to provide
more detail in the report.

Arguments:

* `message`: Message to output.

### `void error(std::string const & label, std::string const & message)`
Reports that an error occurred in the test case.  Errors may include exceeding
instruction execution limit or running into an LC-3 exception.

Arguments:

* `label`: String in the score report to identify this error.
* `msg`: The error message.

### `lc3::core::SymbolTable const & getSymbolTable(void) const`
Access the symbol table as an `std::unordered_map<std::string, uint16_t>`, which
contains the address of each symbol used across all assembly files under test.
If symbols are duplicated across assembly files, only one arbitrary instance
will be accessible in the symbol table.

Return Value:

* The symbol table.

## Automated Input

### `void setInputString(std::string const & source)`
Sets the string that the LC-3 system will use, character by character, when
the KBDR is read.  In general, while there are characters remaining in the
string, the ready bit in the KBSR will be set.  The exception is when
the chararacter delay (see `setInputCharDelay`) is nonzero.

Arguments:

* `source`: String to be used as input by LC-3.

### `void setInputCharDelay(uint32_t char_delay)`
Delays the LC-3 system from reading characters. When the delay is set to `N`,
the ready bit in the KBSR is set every `N` instructions while the input string
has not been fully consumed.  To emulate polling based I/O, it is generally best
to set the delay to 0 so that input is consumed as quickly as possible.  To
emulate interrupt based I/O, it can be advantageous to set the delay to a
nonzero value so that the interrupt service routine has time to complete before
being triggered again.  The delay applies to every character in the input string
and does not change even if the input string does.

Arguments:

* `char_delay`: Delay before the ready bit in in the KBSR is set for each
   character, given in # of instructions executed.

## String Manipulation and Comparison

### `std::string getOutput(void)`
Returns all of the simulated output as a string.

Return Value:

* All of the simulated output as a string.

### `void clearOutput(void)`
Clears the output buffer, which stores the simulated output.

### `bool checkMatch(std::string const & a, std::string const & b)`
Return whether or not two strings equal each other exactly. Generally used to
compare the simulated output (returned from `getOutput`) with the expected
output.

Arguments:

* `a`: One string to compare with.
* `b`: Other string to compare with.

Return Value:

* `true` if the strings match exactly, and `false` otherwise.

### `bool checkContain(std::string const & str, std::string const & expected_part)`
Return whether or not `expected_part` is a substring of `str`. Generally used
to see whether the expected output is within the simulated output (returned from
`getOutput`). Useful when writing unit tests that don't need to compare every
character exactly.

Arguments:

* `str`: Larger string.
* `expected_part`: Substring to check.

Return Value

* `true` if `expected_part` is a substring of `str`, and `false` otherwise.

### `double checkSimilarity(std::string const & a, std::string const & b)`
Return the similarity (computed using Levenshtein Distance) between `source` and
`target` as a percentage within the range of [0-1]. Generally used to see how
similar the expected output is to the simulated output (returned from
`getOutput`). Useful when writing unit tests in which the simulated output only
needs to loosely match the expected output. Note that the implementation is not
very efficient so it should not be used on very long strings.

Arguments:

* `a`: One string to compare similarity with.
* `b`: Other string to compare similarity with.

Return Value:

* Percetage similarity in the range [0-1].

### `enum PreprocessType`
Enumerates preprocessing modes that are supported. Generally used to preprocess
the expected output and/or simulated output (returned from `getOutput`) for
comparison. Useful when writing unit tests in which the simulated output only
needs to match the expected output in content but not necessarily presentation.
Enum values are integers and can be combined together with a bitwise OR (|).

Values:

* `IgnoreCase`: Convert string to lower case.
* `IgnoreWhitespace`: Remove whitespace from string.
* `IgnorePunctuation`: Remove punctuation from string.

### `std::string getPreprocessedString(std::string const & str, uint64_t type)`
Preprocess a string. Generally used to preprocess the expected output and/or
simulated output (returned from `getOutput`) for comparison. Useful when writing
unit tests in which the simulated output only needs to match the expected output
in content but not necessarily presentation. Preprocessing method is supplied in
the `type` argument, generally by converting a `PreprocessType` value to
`uint64_t`. Multiple preprocessing methods can be combined together with a
bitwise OR (|).

Arguments:

* `str`: String to be preprocessed.
* `type`: Preprocessing method to apply.

Return Value:

* Preprocessed string.

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.