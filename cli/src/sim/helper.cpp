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

bool pre_instruction_callback_v = false;
core::callback_func_t pre_instruction_callback;
bool post_instruction_callback_v = false;
core::callback_func_t post_instruction_callback;
bool interrupt_enter_callback_v = false;
core::callback_func_t interrupt_enter_callback;
bool interrupt_exit_callback_v = false;
core::callback_func_t interrupt_exit_callback;

void corePreInstructionCallback(core::MachineState & state);
void corePostInstructionCallback(core::MachineState & state);
void coreInterruptEnterCallback(core::MachineState & state);
void coreInterruptExitCallback(core::MachineState & state);

void coreInit(utils::IPrinter & printer, utils::IInputter & inputter)
{
    interface = new core::lc3(printer, inputter);
    interface->registerPreInstructionCallback(corePreInstructionCallback);
    interface->registerPostInstructionCallback(corePostInstructionCallback);
    interface->registerInterruptEnterCallback(coreInterruptEnterCallback);
    interface->registerInterruptExitCallback(coreInterruptEnterCallback);
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

void corePreInstructionCallback(core::MachineState & state)
{
    for(auto x : breakpoints) {
        if(state.pc == x.loc) {
            std::cout << "hit a breakpoint\n" << x << "\n";
            state.hit_breakpoint = true;
            break;
        }
    }

    if(pre_instruction_callback_v) {
        pre_instruction_callback(state);
    }
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

void corePostInstructionCallback(core::MachineState & state)
{
    inst_exec_count += 1;
    if(limited_run && inst_exec_count == target_inst_count) {
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

uint32_t coreGetReg(uint32_t id)
{
    if(id > 7) {
        throw std::invalid_argument("invalid id");
    }
    return interface->getMachineState().regs[id];
}

uint32_t coreGetPC(void)
{
    return interface->getMachineState().pc;
}

uint32_t coreGetMemVal(uint32_t addr)
{
    if(addr > 0xffff) {
        throw std::invalid_argument("invalid addr");
    }
    return interface->getMachineState().mem[addr].getValue();
}

std::string coreGetMemLine(uint32_t addr)
{
    if(addr > 0xffff) {
        throw std::invalid_argument("invalid addr");
    }
    return interface->getMachineState().mem[addr].getLine();
}

Breakpoint coreSetBreakpoint(uint32_t addr)
{
    if(addr > 0xffff) {
        throw std::invalid_argument("invalid addr");
    }

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

std::ostream & operator<<(std::ostream & out, Breakpoint & x)
{
    out << "#" << x.id << ": " << coreFormatMem(x.loc);
    return out;
}

std::string coreFormatMem(uint32_t addr)
{
    std::stringstream out;
    uint32_t value = coreGetMemVal(addr);
    std::string line = coreGetMemLine(addr);
    out << utils::ssprintf("0x%0.4X: 0x%0.4X %s", addr, value, line.c_str());
    return out.str();
}
