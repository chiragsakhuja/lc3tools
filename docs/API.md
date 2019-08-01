# Grading API
The purpose of this document is to describe the subset of the LC3Tools API that
is relevant to grading. The full API can be found in `src/backend/interface.h`.
It is highly recommended to read the [Graders](GRADE.MD) document before reading
this document.

There are three main components to the API: [`lc3::sim`](API.md#lc3::sim),
[`StringInputter`](API.md#stringinputter), and the [grading
framework](API.md#grading-framework).

# `lc3::sim`
This is the main interface to the simulator and is how common operations, such
as reading and writing memory, are done.  The active `lc3::sim` object is
provided as an argument into each test case.

## Running the Machine

### `void lc3::sim::run(void)`
Run the machine from the location of the PC until completion. Completion is
indicated by setting the clock enable bit in the Machine Status Register,
typically as the last instruction of the HALT trap routine.

### `void lc3::sim::runUntilHalt(void)`
Run the machine from the location of the PC until a HALT (TRAP x25) instruction
is encountered. The HALT trap routine will not run. This is useful when
verifying register contents, as the HALT trap routine may clobber values.

### `void lc3::sim::runUntilInputPoll(void)`
Run the machine from the location of the PC until any keyboard input is
requested. This is useful when verifying I/O.  See the [I/O
Paradigm](GRADE.md#io-paradigm) for more details on usage.

### `void lc3::sim::setRunInstLimit(uint64_t inst_limit)`
Sets the instruction count limit. Regardless of the type of run, simulation will
halt as soon as the instruction limit is reset. This can ensure that the grader
will not run indefinitely even if the program it's running does. The instruction
limit pertains to the next `run*` function executed.

Arguments:

* `inst_limit`: The number of instructions to execute before halting simulation.

## Getting/Setting Machine State

### `uint32_t lc3::sim::getReg(uint32_t id) const`
Get the value of a register.

Arguments:

* `id`: ID of register to read from.

Return Value:

* Value in the register.

### `void lc3::sim::setReg(uint32_t id, uint32_t value)`
Set a register to a value.

Arguments:

* `id`: ID of register to write to.
* `value`: New register value.

### `uint32_t lc3::sim::getMem(uint32_t addr) const`
Get the value of a memory location.

Arguments:

* `addr`: Memory address to read from.

Return Value:

* Value in the memory location.

### `void lc3::sim::setMem(uint32_t id, uint32_t value)`
Set a memory location to a value.

Arguments:

* `id`: Memory address to write to.
* `value`: New memory location value.

# Grading Framework