**Disclaimer:** This document reflects the API for version v1.0.6, which is
deprecated. To see details on the latest API, see [this document](API1.md).

# Grading API
The purpose of this document is to describe the subset of the LC3Tools API that
is relevant to grading. The full API can be found in `src/backend/interface.h`.
It is highly recommended to read the [Graders](GRADE.md) document before reading
this document.

There are three main components to the API: [`lc3::sim`](API.md#lc3sim),
[`StringInputter`](API.md#stringinputter), and the [grading
framework](API.md#grading-framework).

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

### `bool runUntilInputPoll(void)`
Run the machine from the location of the PC until any keyboard input is
requested. This is useful when verifying I/O.  See the [I/O
Paradigm (Polling)](GRADE.md#io-paradigm-polling) and [I/O Paradigm
(Interrupt)](GRADE.md#io-paradigm-interrupt) for more details on usage.

Return Value:

* `true` if program halted without any exceptions, `false` otherwise.

### `void setRunInstLimit(uint64_t inst_limit)`
Sets the instruction count limit. Regardless of the type of run, simulation will
halt as soon as the instruction limit is reset. This can ensure that the grader
will not run indefinitely even if the program it's running does. The instruction
limit pertains to the next `run*` function executed.

Arguments:

* `inst_limit`: The number of instructions to execute before halting simulation.

## Getting/Setting Machine State

### `uint16_t getReg(uint16_t id) const`
Get the value of a register.

Arguments:

* `id`: ID of register to read from.

Return Value:

* Value in the register.

### `void setReg(uint16_t id, uint16_t value)`
Set a register to a value.

Arguments:

* `id`: ID of register to write to.
* `value`: New register value.

### `uint16_t getMem(uint16_t addr) const`
Get the value of a memory location.

Arguments:

* `addr`: Memory address to read from.

Return Value:

* Value in the memory location.

### `void setMem(uint16_t id, uint16_t value)`
Set a memory location to a value.

Arguments:

* `id`: Memory address to write to.
* `value`: New memory location value.

### `uint16_t getPC(void) const`
Get the value of the Program Counter (PC).

Return Value:

* Value of the PC.

### `void setPC(uint16_t value)`
Set the value of the Program Counter (PC).

Arguments:

* `value`: New PC value.

### `uint16_t getPSR(void) const`
Get the value of the Processor Status Register (PSR).

Return Value:

* Value of the PSR.

### `void setPSR(uint16_t value)`
Set the value of the Processor Status Register (PSR).

Arguments:

* `value`: New PSR value.

### `uint16_t getMCR(void) const`
Get the value of the Memory Control Register (MCR).

Return Value:

* Value of the MCR.

### `void setMCR(uint16_t value)`
Set the value of the Memory Control Register (MCR).

Arguments:

* `value`: New MCR value.

### `char getCC(void) const`
Get the value of the Condition Codes (CC) as a character.

Return Value:

* Value of the CC as 'N', 'Z', or 'P'.

### `void setCC(uint16_t value)`
Set the value of the Condition Codes (CC) as a character.

Arguments:

* `value`: New MCR value as 'N', 'Z', or 'P'.

### `void setMemString(uint16_t addr, std::string const & value)`
Set a series of memory locations to a null-terminated string.

Arguments:

* `addr`: Starting address of string.
* `value`: New value of memory locations.

## Callbacks
There are several hooks available that may be useful during grading for things
such as counting the number of times a specific subroutine is called. All
callback functions must match the signature defined by `callback_func_t`.

### `callback_func_t`
The callback function protoype that all callback functions must follow, defined
as `std::function<void(core::MachineState &)>`.

### `void registerPreInstructionCallback(callback_func_t func)`
Register a function that is called before each instruction execution.

Arguments:

* `func`: Function to call before each instruction execution.

### `void registerPostInstructionCallback(callback_func_t func)`
Register a function that is called after each instruction execution.

Arguments:

* `func`: Function to call after each instruction execution.

### `void registerSubEnterCallback(callback_func_t func)`
Register a function that is called before entering a subroutine.

Arguments:

* `func`: Function to call before entering a subroutine.


### `void registerSubExitCallback(callback_func_t func)`
Register a function that is called after exiting a subroutine.

Arguments:

* `func`: Function to call after exiting a subroutine.

### `void registerInterruptEnterCallback(callback_func_t func)`
Register a function that is called before an interrupt service routine begins.

Arguments:

* `func`: Function to call before an interrupt service routine begins.

### `void registerInterruptExitCallback(callback_func_t func)`
Register a function that is called after an interrupt service routine ends.

Arguments:

* `func`: Function to call after an interrupt service routine ends.

## Miscellaneous

### `bool didExceedInstLimit(void) const`
Check if instruction limit was exceeded. This is useful for verifying that a
program halted properly.

Return Value:

* `true` if the instruction limit was exceeded, `false` otherwise.

# `StringInputter`
This object is used to set the keyboard input values that the program will
consume. The `StringInputter` object is provided as an argument into each test
case. See the [I/O Paradigm](GRADE.md#io-paradigm) for more details on usage.

### `void setString(std::string const & source)`
Set the string that the simulator will consume through the keyboard emulator.

Arguments:

* `source`: The string that the simulator will consume through the keyboard
emulator.

### `void setStringAfter(std::string const & source, uint32_t inst_count)`
Set the string that the simulator will consume through the keyboard emulator,
but only after `inst_count` instructions have been executed. This is useful for
verifying interrupts, as it takes some initial instructions to set up
interrupts.

Arguments:

* `source`: The string that the simulator will consume through the keyboard
emulator.
* `inst_count`: The number of instructions that execute before the keyboard
press is emulated.

# Grading Framework
Additionally, the grading framework provides a set of macros that should be used
for verifying programs.

### `REGISTER_TEST(name, function, points)`
Register a test case.

Arguments:

* `name`: Name of the test case on the report.
* `function`: Function for the test case. Must have the signature 
`void(lc3::sim &, StringInputter &)`.
* `points`: Total number of points allocated to the test case.

### `REGISTER_RANDOM_TEST(name, function, points)`
Register a test case, but randomize the machine before running it.

Arguments:

* `name`: Name of the test case on the report.
* `function`: Function for the test case. Must have the signature 
`void(lc3::sim &, StringInputter &)`.
* `points`: Total number of points allocated to the test case.

### `VERIFY(check)`
Checks a predicate and adds points if `true`. The number of points assigned
are split evenly across all `VERIFY` statements in a test case.

Arguments:

* `check`: Predicate to be checked.

### `VERIFY_NAMED(message, check)`
Checks a predicate and adds points if `true`. The number of points assigned
are split evenly across all `VERIFY` statements in a test case.

Arguments:

* `message`: Message to display on the report.
* `check`: Predicate to be checked.

### `VERIFY_OUTPUT(check)`
Checks if the display output matches a string and add points if exact match. The 
number of points assigned are split evenly across all `VERIFY` statements in a
test case.

Arguments:

* `check`: String to compare output to.

### `VERIFY_OUTPUT_NAMED(message, check)`
Checks if the display output matches a string and add points if exact match. The 
number of points assigned are split evenly across all `VERIFY` statements in a
test case.

Arguments:

* `message`: Message to display on the report.
* `check`: String to compare output to.

### `VERIFY_OUTPUT_HAD(check)`
Checks if any part of the display output matches a string and add points if
so. The number of points assigned are split evenly across all `VERIFY`
statements in a test case.

Arguments:

* `check`: String to compare output to.

### `VERIFY_OUTPUT_HAD_NAMED(message, check)`
Checks if any part of the display output matches a string and add points if
so. The number of points assigned are split evenly across all `VERIFY`
statements in a test case.

Arguments:

* `message`: Message to display on the report.
* `check`: String to compare output to.

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.