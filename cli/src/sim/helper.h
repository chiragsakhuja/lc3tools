#ifndef HELPER_H
#define HELPER_H

#include "core.h"

struct Breakpoint { uint32_t id, loc; };

extern core::lc3 * interface;
extern uint32_t inst_exec_count;
extern std::vector<Breakpoint> breakpoints;

void coreInit(utils::IPrinter & printer, utils::IInputter & inputter);
void coreShutdown(void);
void coreLoadSimulatorWithFile(std::string const & filename);
bool coreRun(void);
bool coreRunFor(uint32_t inst_count);
void coreRegisterPreInstructionCallback(core::callback_func_t func);
void coreRegisterPostInstructionCallback(core::callback_func_t func);
void coreRegisterInterruptEnterCallback(core::callback_func_t func);
void coreRegisterInterruptExitCallback(core::callback_func_t func);
void coreRegisterBreakpointHitCallback(std::function<void(core::MachineState & state, Breakpoint const & bp)> func);

uint32_t coreGetReg(uint32_t id);
uint32_t coreGetPC(void);
uint32_t coreGetMemVal(uint32_t addr);
std::string coreGetMemLine(uint32_t addr);

Breakpoint coreSetBreakpoint(uint32_t addr);
bool coreRemoveBreakpoint(uint32_t id);

#endif
