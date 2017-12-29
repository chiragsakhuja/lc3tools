#include <cassert>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <sstream>

#include "helper.h"
#include "console_printer.h"

core::lc3 * interface = nullptr;

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

void simPreInstructionCallback(core::MachineState & state);
void simPostInstructionCallback(core::MachineState & state);
void simInterruptEnterCallback(core::MachineState & state);
void simInterruptExitCallback(core::MachineState & state);
void simSubEnterCallback(core::MachineState & state);
void simSubExitCallback(core::MachineState & state);

class NullInputter : public utils::IInputter
{
    public:
        virtual void beginInput(void) override {}
        virtual bool getChar(char & c) override { return false; }
        virtual void endInput(void) override {}
};

std::string assemble(std::string const & asm_filename)
{
    NullInputter inputter;
    utils::ConsolePrinter printer;

    std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");

    interface = new core::lc3(printer, inputter);
    interface->assemble(asm_filename, obj_filename);
    delete interface;

    return obj_filename;
}

void simInit(utils::IPrinter & printer, utils::IInputter & inputter)
{
    interface = new core::lc3(printer, inputter);
    interface->registerPreInstructionCallback(simPreInstructionCallback);
    interface->registerPostInstructionCallback(simPostInstructionCallback);
    interface->registerInterruptEnterCallback(simInterruptEnterCallback);
    interface->registerInterruptExitCallback(simInterruptExitCallback);
    interface->registerSubEnterCallback(simSubEnterCallback);
    interface->registerSubExitCallback(simSubExitCallback);
    interface->initializeSimulator();
}

void simRandomizeMachine(void)
{
    assert(interface != nullptr);

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> dis(0x0000, 0xffff);

    for(uint32_t i = 0x0; i < 0xfe00; i += 1) {
        simSetMemVal(i, dis(gen));
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        simSetReg(i, dis(gen));
    }

    interface->initializeSimulator();
}

void simShutdown(void)
{
    assert(interface != nullptr);

    delete interface;
}

bool simLoadSimulatorWithFile(std::string const & filename)
{
    assert(interface != nullptr);

    try {
        interface->loadSimulatorWithFile(filename);
    } catch (utils::exception const & e) {
        return false;
    }

    return true;
}

bool simRun(void)
{
    assert(interface != nullptr);

    limited_run = false;
    try {
        interface->simulate();
    } catch(utils::exception const & e) {
        return false;
    }
    return true;
}

bool simRunFor(uint32_t inst_count)
{
    assert(interface != nullptr);

    target_inst_count = inst_exec_count + inst_count;
    limited_run = true;
    try {
        interface->simulate();
    } catch(utils::exception const & e) {
        return false;
    }
    return true;
}

bool simStepOver(void)
{
    assert(interface != nullptr);

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

bool simStepOut(void)
{
    assert(interface != nullptr);

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

void simRegisterPreInstructionCallback(core::callback_func_t func)
{
    pre_instruction_callback_v = true;
    pre_instruction_callback = func;
}

void simRegisterPostInstructionCallback(core::callback_func_t func)
{
    post_instruction_callback_v = true;
    post_instruction_callback = func;
}

void simRegisterInterruptEnterCallback(core::callback_func_t func)
{
    interrupt_enter_callback_v = true;
    interrupt_enter_callback = func;
}


void simRegisterInterruptExitCallback(core::callback_func_t func)
{
    interrupt_exit_callback_v = true;
    interrupt_exit_callback = func;
}

void simRegisterSubEnterCallback(core::callback_func_t func)
{
    sub_enter_callback_v = true;
    sub_enter_callback = func;
}


void simRegisterSubExitCallback(core::callback_func_t func)
{
    sub_exit_callback_v = true;
    sub_exit_callback = func;
}

void simRegisterBreakpointHitCallback(std::function<void(core::MachineState & state, Breakpoint const & bp)> func)
{
    breakpoint_hit_callback_v = true;
    breakpoint_hit_callback = func;
}

void simPreInstructionCallback(core::MachineState & state)
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

void simPostInstructionCallback(core::MachineState & state)
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

void simInterruptEnterCallback(core::MachineState & state)
{
    if(interrupt_enter_callback_v) {
        interrupt_enter_callback(state);
    }
}

void simInterruptExitCallback(core::MachineState & state)
{
    if(interrupt_exit_callback_v) {
        interrupt_exit_callback(state);
    }
}

void simSubEnterCallback(core::MachineState & state)
{
    sub_depth += 1;
    if(sub_enter_callback_v) {
        sub_enter_callback(state);
    }
}

void simSubExitCallback(core::MachineState & state)
{
    sub_depth -= 1;
    if(sub_exit_callback_v) {
        sub_exit_callback(state);
    }
}

uint32_t simGetReg(uint32_t id)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert(id <= 7);
#else
    id &= 0x7;
#endif
    return interface->getMachineState().regs[id];
}

uint32_t simGetPC(void) { assert(interface != nullptr); return interface->getMachineState().pc; }
uint32_t simGetPSR(void) { assert(interface != nullptr); return interface->getMachineState().psr; }

char simGetCC(void) {
    assert(interface != nullptr);

    uint32_t cc = simGetPSR() & 0x7;
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

uint32_t simGetMemVal(uint32_t addr)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return interface->getMachineState().mem[addr].getValue();
}

std::string simGetMemLine(uint32_t addr)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return interface->getMachineState().mem[addr].getLine();
}

void simSetReg(uint32_t id, uint32_t value)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert(id <= 7);
    assert(value <= 0xffff);
#else
    id &= 0x7;
    value &= 0xffff;
#endif
    interface->getMachineState().regs[id] = value;
}

void simSetPC(uint32_t value)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert(value <= 0xfe00u);
#else
    value = std::min(value & 0xffff, 0xfe00u);
#endif
    interface->getMachineState().pc = value;

    // re-enable system
    uint32_t mcr = interface->getMachineState().mem[0xfffe].getValue();
    interface->getMachineState().mem[0xfffe].setValue(mcr | 0x8000);
}

void simSetPSR(uint32_t value)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8707) & 0xffff)) == 0x0000);
#else
    value &= ((~0x8707) & 0xffff);
#endif
    interface->getMachineState().psr = value;
}

void simSetCC(char cc)
{
    assert(interface != nullptr);

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

void simSetMemVal(uint32_t addr, uint32_t value)
{
    assert(interface != nullptr);

#ifdef _ENABLE_DEBUG
    assert(addr <= 0xfe00);
    assert(value <= 0xffff);
#else
    addr = std::min(addr & 0xffffu, 0xfe00u);
    value &= 0xffff;
#endif
    interface->getMachineState().mem[addr].setValue(value);
}

Breakpoint simSetBreakpoint(uint32_t addr)
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

bool simRemoveBreakpoint(uint32_t id)
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
