#include <fstream>
#include <mutex>
#include <thread>
#include <sstream>

#include "device_regs.h"
#include "simulator.h"
#include "utils.h"

using namespace lc3::core;

namespace lc3
{
namespace core
{
    std::mutex g_io_lock;
};
};

Simulator::Simulator(lc3::sim & simulator, lc3::utils::IPrinter & printer, lc3::utils::IInputter & inputter,
    uint32_t print_level) : state(simulator, logger), logger(printer, print_level), inputter(inputter),
    collecting_input(false)
{
    state.mem.resize(1 << 16);
    reinitialize();

    state.pre_instruction_callback_v = false;
    state.post_instruction_callback_v = false;
    state.interrupt_enter_callback_v = false;
    state.interrupt_exit_callback_v = false;
}

void Simulator::loadObjectFile(std::string const & filename)
{
    std::ifstream file(filename);
    if(! file) {
        logger.printf(lc3::utils::PrintType::P_ERROR, true, "could not open file \'%s\' for reading",
            filename.c_str());
        throw utils::exception("could not open file");
    }

    loadObjectFileFromBuffer(file);
}

void Simulator::loadObjectFileFromBuffer(std::istream & buffer)
{
    uint32_t fill_pc = 0;
    uint32_t offset = 0;
    bool first_orig_set = false;
    while(! buffer.eof()) {
        MemEntry statement;
        buffer >> statement;

        if(buffer.eof()) {
            break;
        }

        if(statement.isOrig()) {
            if(! first_orig_set) {
                state.pc = statement.getValue();
                first_orig_set = true;
            }
            fill_pc = statement.getValue();
            offset = 0;
        } else {
            logger.printf(lc3::utils::PrintType::P_DEBUG, true, "0x%0.4x: %s (0x%0.4x)", fill_pc + offset,
                statement.getLine().c_str(), statement.getValue());
            state.mem[fill_pc + offset] = statement;
            offset += 1;
        }

    }
    uint16_t value = state.readMemRaw(MCR);
    state.writeMemRaw(MCR, value | 0x8000);
}

void Simulator::loadOS()
{
    extern char const * lc3os_src;

    std::stringstream buffer;
    buffer << lc3os_src;
    loadObjectFileFromBuffer(buffer);
    state.pc = RESET_PC;
}

void Simulator::simulate(void)
{
    std::thread input_thread;
    utils::exception exception;
    bool exception_valid = false;

    try {
        state.running = true;
        collecting_input = true;
        inputter.beginInput();

        input_thread = std::thread(&core::Simulator::handleInput, this);
        while(state.running && (state.readMemRaw(MCR) & 0x8000) != 0) {
            if(! state.hit_breakpoint) {
                executeEvent(std::make_shared<CallbackEvent>(state.pre_instruction_callback_v,
                    state.pre_instruction_callback));
                if(state.hit_breakpoint) {
                    break;
                }
            }
            state.hit_breakpoint = false;

            std::vector<PIEvent> events = executeInstruction();
            events.push_back(std::make_shared<CallbackEvent>(state.post_instruction_callback_v,
                state.post_instruction_callback));
            executeEventChain(events);
            updateDevices();
            events = checkAndSetupInterrupts();
            executeEventChain(events);
        }
    } catch(utils::exception & e) {
        exception = e;
        exception_valid = true;
    } catch(std::exception & e) { }

    state.running = false;
    collecting_input = false;
    if(input_thread.joinable()) {
        input_thread.join();
    }
    inputter.endInput();

    if(exception_valid) {
        throw exception;
    }
}

void Simulator::pause(void)
{
    state.running = false;
}

std::vector<PIEvent> Simulator::executeInstruction(void)
{
    if((state.pc <= SYSTEM_END || state.pc >= MMIO_START) && (state.readMemRaw(PSR) & 0x8000) == 0x8000) {
        logger.printf(lc3::utils::PrintType::P_EXTRA, true, "illegal PC 0x%0.4x accessed", state.pc);
        return IInstruction::buildSysCallEnterHelper(state, INTEX_TABLE_START + 0x0, MachineState::SysCallType::INT);
    }
    uint32_t encoded_inst = state.readMemSafe(state.pc);

    bool valid;
    PIInstruction candidate = decoder.findInstructionByEncoding(encoded_inst, valid);
    if(! valid) {
        logger.printf(lc3::utils::PrintType::P_EXTRA, true, "illegal opcode");
        return IInstruction::buildSysCallEnterHelper(state, INTEX_TABLE_START + 0x1, MachineState::SysCallType::INT);
    }

    candidate->assignOperands(encoded_inst);
    logger.printf(lc3::utils::PrintType::P_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc,
        state.mem[state.pc].getLine().c_str(), encoded_inst);
    state.pc = (state.pc + 1) & 0xffff;
    std::vector<PIEvent> events = candidate->execute(state);

    return events;
}

void Simulator::updateDevices(void)
{
    uint16_t value = state.readMemRaw(DSR);
    state.writeMemRaw(DSR, value | 0x8000);
}

std::vector<PIEvent> Simulator::checkAndSetupInterrupts(void)
{
    uint32_t value = state.readMemRaw(KBSR);

    if((value & 0xc000) == 0xc000) {
        logger.printf(lc3::utils::PrintType::P_EXTRA, true, "jumping to keyboard ISR");

        std::vector<PIEvent> ret = IInstruction::buildSysCallEnterHelper(state, INTEX_TABLE_START + 0x80,
            MachineState::SysCallType::INT, [](uint32_t psr_value) { return (psr_value & 0x78ff) | 0x0400; });
        ret.push_back(std::make_shared<MemWriteEvent>(KBSR, state.readMemRaw(KBSR) & 0x7fff));
        return ret;
    }

    return {};
}

void Simulator::executeEventChain(std::vector<PIEvent> & events)
{
    for(PIEvent event : events) {
        executeEvent(event);
    }

    events.clear();
}

void Simulator::executeEvent(PIEvent event)
{
    if(event->getOutputString(state) != "") {
        logger.printf(lc3::utils::PrintType::P_EXTRA, false, "  %s", event->getOutputString(state).c_str());
    }
    event->updateState(state);
}

void Simulator::reinitialize(void)
{
    for(uint32_t i = 0; i < 8; i += 1) {
        state.regs[i] = 0;
    }

    state.pc = RESET_PC;

    for(uint32_t i = 0; i < (1 << 16); i += 1) {
        state.writeMemRaw(i, 0);
        state.mem[i].setLine("");
    }

    state.writeMemRaw(BSP, 0x3000);
    state.writeMemRaw(PSR, 0x8002);
    state.writeMemRaw(MCR, 0x8000);  // indicate the machine is running
    state.running = true;
    state.hit_breakpoint = false;
}

void Simulator::registerPreInstructionCallback(callback_func_t func)
{
    state.pre_instruction_callback_v = true;
    state.pre_instruction_callback = func;
}

void Simulator::registerPostInstructionCallback(callback_func_t func)
{
    state.post_instruction_callback_v = true;
    state.post_instruction_callback = func;
}

void Simulator::registerInterruptEnterCallback(callback_func_t func)
{
    state.interrupt_enter_callback_v = true;
    state.interrupt_enter_callback = func;
}

void Simulator::registerInterruptExitCallback(callback_func_t func)
{
    state.interrupt_exit_callback_v = true;
    state.interrupt_exit_callback = func;
}

void Simulator::registerSubEnterCallback(callback_func_t func)
{
    state.sub_enter_callback_v = true;
    state.sub_enter_callback = func;
}

void Simulator::registerSubExitCallback(callback_func_t func)
{
    state.sub_exit_callback_v = true;
    state.sub_exit_callback = func;
}

void Simulator::handleInput(void)
{
    while(collecting_input) {
        char c;
        if(inputter.getChar(c)) {
            std::lock_guard<std::mutex> guard(g_io_lock);
            state.writeMemRaw(KBSR, state.readMemRaw(KBSR) | 0x8000);
            state.writeMemRaw(KBDR, ((uint32_t) c) & 0xFF);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
