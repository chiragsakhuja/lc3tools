#include <array>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "configured_paths.h"

#include "utils.h"

#include "tokens.h"

#include "inputter.h"

#include "printer.h"
#include "logger.h"

#include "statement.h"

#include "state.h"

#include "instructions.h"
#include "instruction_decoder.h"

#include "device_regs.h"

#include "simulator.h"

namespace core {
    std::mutex g_io_lock;
};

core::Simulator::Simulator(bool log_enable, utils::IPrinter & printer, utils::IInputter & inputter) :
    pre_instruction_callback_v(false), post_instruction_callback_v(false), interrupt_enter_callback_v(false),
    interrupt_exit_callback_v(false), state(logger), logger(log_enable, printer), inputter(inputter),
    collecting_input(false)
{
    state.mem.resize(1 << 16);
    reset();
}

void core::Simulator::loadObjectFile(std::string const & filename)
{
    std::ifstream file(filename);
    if(! file) {
        logger.printf(PRINT_TYPE_ERROR, true, "could not open file \'%s\' for reading", filename.c_str());
        throw utils::exception("could not open file");
    }

    uint32_t offset = 0;
    while(! file.eof()) {
        utils::Statement statement;
        file >> statement;

        if(file.eof()) {
            break;
        }

        if(statement.isOrig()) {
            state.pc = statement.getValue();
            offset = 0;
        } else {
            logger.printf(PRINT_TYPE_DEBUG, true, "0x%0.4x: %s (0x%0.4x)", state.pc + offset,
                statement.getLine().c_str(), statement.getValue());
            state.mem[state.pc + offset] = statement;
            offset += 1;
        }

    }
    uint16_t value = state.mem[MCR].getValue();
    state.mem[MCR].setValue(value | 0x8000);
}

void core::Simulator::loadOS(void)
{
    loadObjectFile("lc3os.obj");
    state.pc = RESET_PC;
}

void core::Simulator::simulate(void)
{
    state.running = true;
    collecting_input = true;
    inputter.beginInput();
    std::thread input_thread(&core::Simulator::handleInput, this);

    while(state.running && (state.mem[MCR].getValue() & 0x8000) != 0) {
        if(!state.hit_breakpoint && pre_instruction_callback_v) {
            pre_instruction_callback(state);
            // in case the pre instruction callback wants to disable simulation
            if(state.hit_breakpoint) {
                break;
            }
        }
        state.hit_breakpoint = false;

        std::vector<IEvent const *> events = executeInstruction();
        executeEventChain(events);
        updateDevices();
        if(post_instruction_callback_v) {
            post_instruction_callback(state);
        }

        bool interrupt_triggered;
        events = checkAndSetupInterrupts(interrupt_triggered);
        if(interrupt_triggered) {
            executeEventChain(events);
            if(interrupt_enter_callback_v) {
                interrupt_enter_callback(state);
            }
        }
    }

    state.running = false;
    collecting_input = false;
    input_thread.join();
    inputter.endInput();
}

std::vector<core::IEvent const *> core::Simulator::executeInstruction(void)
{
    uint32_t encoded_inst = state.mem[state.pc].getValue();

    IInstruction * candidate;
    if(! decoder.findInstructionByEncoding(encoded_inst, candidate)) {
        logger.printf(PRINT_TYPE_ERROR, true, "invalid instruction 0x%0.4x", encoded_inst);
        throw utils::exception("invalid instruction");
    }

    candidate->assignOperands(encoded_inst);
    logger.printf(PRINT_TYPE_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc,
        state.mem[state.pc].getLine().c_str(), encoded_inst);
    state.pc += 1;
    std::vector<IEvent const *> events = candidate->execute(state);
    delete candidate;

    return events;
}

void core::Simulator::updateDevices(void)
{
    uint16_t value = state.mem[DSR].getValue();
    state.mem[DSR].setValue(value | 0x8000);
}

std::vector<core::IEvent const *> core::Simulator::checkAndSetupInterrupts(bool & interrupt_triggered)
{
    std::vector<IEvent const *> ret;

    uint32_t value = state.mem[KBSR].getValue();
    interrupt_triggered = false;
    if((value & 0xC000) == 0xC000) {
        ret.push_back(new RegEvent(6, state.regs[6] - 1));
        ret.push_back(new MemWriteEvent(state.regs[6] - 1, state.psr));
        ret.push_back(new RegEvent(6, state.regs[6] - 2));
        ret.push_back(new MemWriteEvent(state.regs[6] - 2, state.pc));
        ret.push_back(new PSREvent((state.psr & 0x78FF) | 0x0400));
        ret.push_back(new PCEvent(state.mem[0x0180].getValue()));
        ret.push_back(new MemWriteEvent(KBSR, state.mem[KBSR].getValue() & 0x7fff));
        interrupt_triggered = true;
    }

    return ret;
}

void core::Simulator::executeEventChain(std::vector<core::IEvent const *> & events)
{
    for(uint32_t i = 0; i < events.size(); i += 1) {
        logger.printf(PRINT_TYPE_EXTRA, false, "  %s", events[i]->getOutputString(state).c_str());
        events[i]->updateState(state);

        delete events[i];
    }

    events.clear();
}

void core::Simulator::reset(void)
{
    for(uint32_t i = 0; i < 8; i += 1) {
        state.regs[i] = 0;
    }

    state.pc = RESET_PC;
    state.psr = 0x8002;
    state.backup_sp = 0x3000;

    for(uint32_t i = 0; i < (1 << 16); i += 1) {
        state.mem[i].setValue(0);
    }

    state.mem[MCR].setValue(0x8000);  // indicate the machine is running
    state.running = true;
    state.hit_breakpoint = false;
}

void core::Simulator::registerPreInstructionCallback(std::function<void(MachineState & state)> func)
{
    pre_instruction_callback_v = true;
    pre_instruction_callback = func;
}

void core::Simulator::registerPostInstructionCallback(std::function<void(MachineState & state)> func)
{
    post_instruction_callback_v = true;
    post_instruction_callback = func;
}

void core::Simulator::registerInterruptEnterCallback(std::function<void(MachineState & state)> func)
{
    interrupt_enter_callback_v = true;
    interrupt_enter_callback = func;
}

void core::Simulator::registerInterruptExitCallback(std::function<void(MachineState & state)> func)
{
    interrupt_exit_callback_v = true;
    interrupt_exit_callback = func;
}

void core::Simulator::handleInput(void)
{
    while(collecting_input) {
        char c;
        if(inputter.getChar(c)) {
            std::lock_guard<std::mutex> guard(g_io_lock);
            state.mem[KBSR].setValue(state.mem[KBSR].getValue() | 0x8000);
            state.mem[KBDR].setValue(((uint32_t) c) & 0xFF);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
