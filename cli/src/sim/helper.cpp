#include <cassert>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "helper.h"

core::lc3 * interface = 0;

uint32_t inst_exec_count = 0;
uint32_t target_inst_count = 0;
bool limited_run = false;

uint32_t breakpoint_id = 0;
std::vector<Breakpoint> breakpoints;
int32_t sub_depth = 0;

bool pre_instruction_callback_v = false;
core::callback_func_t pre_instruction_callback;
bool post_instruction_callback_v = false;
core::callback_func_t post_instruction_callback;
bool interrupt_enter_callback_v = false;
core::callback_func_t interrupt_enter_callback;
bool interrupt_exit_callback_v = false;
core::callback_func_t interrupt_exit_callback;
bool breakpoint_hit_callback_v = false;
bool sub_enter_callback_v = false;
core::callback_func_t sub_enter_callback;
bool sub_exit_callback_v = false;
core::callback_func_t sub_exit_callback;
std::function<void(core::MachineState & state, Breakpoint const & bp)> breakpoint_hit_callback;

void corePreInstructionCallback(core::MachineState & state);
void corePostInstructionCallback(core::MachineState & state);
void coreInterruptEnterCallback(core::MachineState & state);
void coreInterruptExitCallback(core::MachineState & state);
void coreSubEnterCallback(core::MachineState & state);
void coreSubExitCallback(core::MachineState & state);

void coreInit(utils::IPrinter & printer, utils::IInputter & inputter)
{
    interface = new core::lc3(printer, inputter);
    interface->registerPreInstructionCallback(corePreInstructionCallback);
    interface->registerPostInstructionCallback(corePostInstructionCallback);
    interface->registerInterruptEnterCallback(coreInterruptEnterCallback);
    interface->registerInterruptExitCallback(coreInterruptExitCallback);
    interface->registerSubEnterCallback(coreSubEnterCallback);
    interface->registerSubExitCallback(coreSubExitCallback);
    interface->initializeSimulator();
}

void coreShutdown(void)
{
    if(interface != 0) {
        delete interface;
    }
}

void coreLoadSimulatorWithFile(std::string const & filename)
{
    try {
        interface->loadSimulatorWithFile(filename);
    } catch (utils::exception const & e) {}
}

bool coreRun(void)
{
    limited_run = false;
    try {
        interface->simulate();
    } catch(utils::exception const & e) {
        return false;
    }
    return true;
}

bool coreRunFor(uint32_t inst_count)
{
    target_inst_count = inst_exec_count + inst_count;
    limited_run = true;
    try {
        interface->simulate();
    } catch(utils::exception const & e) {
        return false;
    }
    return true;
}

bool coreStepOver(void)
{
    limited_run = true;
    // this will immediately be incremented by the sub enter callback
    sub_depth = 0;

    try {
        interface->simulate();
    } catch(utils::exception const & e) {
        return false;
    }
    return true;
}

bool coreStepOut(void)
{
    limited_run = true;
    // act like we are already in a subroutine
    sub_depth = 1;
    try {
        interface->simulate();
    } catch(utils::exception const & e) {
        return false;
    }
    return true;
}

void coreRegisterPreInstructionCallback(core::callback_func_t func)
{
    pre_instruction_callback_v = true;
    pre_instruction_callback = func;
}

void coreRegisterPostInstructionCallback(core::callback_func_t func)
{
    post_instruction_callback_v = true;
    post_instruction_callback = func;
}

void coreRegisterInterruptEnterCallback(core::callback_func_t func)
{
    interrupt_enter_callback_v = true;
    interrupt_enter_callback = func;
}


void coreRegisterInterruptExitCallback(core::callback_func_t func)
{
    interrupt_exit_callback_v = true;
    interrupt_exit_callback = func;
}

void coreRegisterSubEnterCallback(core::callback_func_t func)
{
    sub_enter_callback_v = true;
    sub_enter_callback = func;
}


void coreRegisterSubExitCallback(core::callback_func_t func)
{
    sub_exit_callback_v = true;
    sub_exit_callback = func;
}

void coreRegisterBreakpointHitCallback(std::function<void(core::MachineState & state, Breakpoint const & bp)> func)
{
    breakpoint_hit_callback_v = true;
    breakpoint_hit_callback = func;
}

void corePreInstructionCallback(core::MachineState & state)
{
    for(auto const & x : breakpoints) {
        if(state.pc == x.loc) {
            if(breakpoint_hit_callback_v) {
                breakpoint_hit_callback(state, x);
            }
            state.hit_breakpoint = true;
            break;
        }
    }

    if(pre_instruction_callback_v) {
        pre_instruction_callback(state);
    }
}

void corePostInstructionCallback(core::MachineState & state)
{
    inst_exec_count += 1;
    if(limited_run && (inst_exec_count == target_inst_count || sub_depth == 0)) {
        limited_run = false;
        state.running = false;
    }

    if(post_instruction_callback_v) {
        post_instruction_callback(state);
    }
}

void coreInterruptEnterCallback(core::MachineState & state)
{
    if(interrupt_enter_callback_v) {
        interrupt_enter_callback(state);
    }
}

void coreInterruptExitCallback(core::MachineState & state)
{
    if(interrupt_exit_callback_v) {
        interrupt_exit_callback(state);
    }
}

void coreSubEnterCallback(core::MachineState & state)
{
    sub_depth += 1;
    if(sub_enter_callback_v) {
        sub_enter_callback(state);
    }
}

void coreSubExitCallback(core::MachineState & state)
{
    sub_depth -= 1;
    if(sub_exit_callback_v) {
        sub_exit_callback(state);
    }
}

uint32_t coreGetReg(uint32_t id)
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
#else
    id &= 0x7;
#endif
    return interface->getMachineState().regs[id];
}

uint32_t coreGetPC(void) { return interface->getMachineState().pc; }
uint32_t coreGetPSR(void) { return interface->getMachineState().psr; }

char coreGetCC(void) {
    uint32_t cc = coreGetPSR() & 0x7;
#ifdef _ENABLE_DEBUG
    assert(cc == 0x4 || cc == 0x2 || cc == 0x1);
#else
    if(cc == 0x4 || cc == 0x2 || cc == 0x1) {
        cc = 0x1;
    }
#endif
    if(cc == 0x4) { return 'N'; }
    else if(cc == 0x2) { return 'Z'; }
    else { return 'P'; }
}

uint32_t coreGetMemVal(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return interface->getMachineState().mem[addr].getValue();
}

std::string coreGetMemLine(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return interface->getMachineState().mem[addr].getLine();
}

void coreSetReg(uint32_t id, uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
    assert(value <= 0xffff);
#else
    id &= 0x7;
    value &= 0xffff;
#endif
    interface->getMachineState().regs[id] = value;
}

void coreSetPC(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(value <= 0xfe00u);
#else
    value = std::min(value & 0xffff, 0xfe00u);
#endif
    interface->getMachineState().pc = value;
}

void coreSetPSR(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8707) & 0xffff)) == 0x0000);
#else
    value &= ((~0x8707) & 0xffff);
#endif
    interface->getMachineState().psr = value;
}

void coreSetCC(char cc)
{
    cc |= 0x20;
#ifdef _ENABLE_DEBUG
    assert(cc == 'n' || cc == 'z' || cc == 'p');
#else
    if(! (cc == 'n' || cc == 'z' || cc == 'p')) { return; }
#endif
    uint32_t new_cc = 0;
    if(cc == 'n') { new_cc = 0x4; }
    else if(cc == 'z') { new_cc = 0x2; }
    else { new_cc = 0x1; }
    uint32_t & psr = interface->getMachineState().psr;
    psr = (psr & 0xfff8) | new_cc;
}

void coreSetMemVal(uint32_t addr, uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xfe00);
    assert(value <= 0xffff);
#else
    addr = std::min(addr & 0xffffu, 0xfe00u);
    value &= 0xffff;
#endif
    interface->getMachineState().mem[addr].setValue(value);
}

Breakpoint coreSetBreakpoint(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    Breakpoint bp{breakpoint_id, addr};
    breakpoints.push_back(bp);
    breakpoint_id += 1;
    return bp;
}

bool coreRemoveBreakpoint(uint32_t id)
{
    auto it = breakpoints.begin();
    bool found = false;
    for(; it != breakpoints.end(); ++it) {
        if(it->id == id) {
            found = true;
            break;
        }
    }

    if(found) {
        breakpoints.erase(it);
    }

    return found;
}
