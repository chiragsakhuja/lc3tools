#ifndef HELPER_H
#define HELPER_H

#include "core.h"

struct Breakpoint { uint32_t id, loc; };

extern core::lc3 * interface;
extern uint32_t inst_exec_count;
extern std::vector<Breakpoint> breakpoints;

std::string assemble(std::string const & asm_filename);

void simInit(utils::IPrinter & printer, utils::IInputter & inputter);
void simRandomizeMachine(void);
void simShutdown(void);
bool simLoadSimulatorWithFile(std::string const & filename);
bool simRun(void);
bool simRunFor(uint32_t inst_count);
bool simStepOver(void);
bool simStepOut(void);
void simRegisterPreInstructionCallback(core::callback_func_t func);
void simRegisterPostInstructionCallback(core::callback_func_t func);
void simRegisterInterruptEnterCallback(core::callback_func_t func);
void simRegisterInterruptExitCallback(core::callback_func_t func);
void simRegisterBreakpointHitCallback(std::function<void(core::MachineState & state, Breakpoint const & bp)> func);

uint32_t simGetReg(uint32_t id);
uint32_t simGetPC(void);
uint32_t simGetPSR(void);
char simGetCC(void);
uint32_t simGetMemVal(uint32_t addr);
std::string simGetMemLine(uint32_t addr);
void simSetReg(uint32_t id, uint32_t value);
void simSetPC(uint32_t value);
void simSetPSR(uint32_t value);
void simSetCC(char cc);
void simSetMemVal(uint32_t addr, uint32_t value);

Breakpoint simSetBreakpoint(uint32_t addr);
bool simRemoveBreakpoint(uint32_t id);

#endif
