#include <fstream>
#include <mutex>
#include <thread>

#include "device_regs.h"
#include "simulator.h"

namespace lc3::core {
    std::mutex g_io_lock;
};

lc3::core::Simulator::Simulator(lc3::sim & simulator, utils::IPrinter & printer, utils::IInputter & inputter,
    uint32_t log_level) : state(simulator, logger), logger(printer, log_level), inputter(inputter),
    collecting_input(false)
{
    state.mem.resize(1 << 16);
    reset();
}

void lc3::core::Simulator::loadObjectFile(std::string const & filename)
{
    std::ifstream file(filename);
    if(! file) {
        logger.printf(lc3::utils::PrintType::PRINT_TYPE_ERROR, true, "could not open file \'%s\' for reading",
            filename.c_str());
        throw utils::exception("could not open file");
    }

    uint32_t offset = 0;
    while(! file.eof()) {
        Statement statement;
        file >> statement;

        if(file.eof()) {
            break;
        }

        if(statement.isOrig()) {
            state.pc = statement.getValue();
            offset = 0;
        } else {
            logger.printf(lc3::utils::PrintType::PRINT_TYPE_DEBUG, true, "0x%0.4x: %s (0x%0.4x)", state.pc + offset,
                statement.getLine().c_str(), statement.getValue());
            state.mem[state.pc + offset] = statement;
            offset += 1;
        }

    }
    uint16_t value = state.mem[MCR].getValue();
    state.mem[MCR].setValue(value | 0x8000);
}

void lc3::core::Simulator::loadOS(void)
{
    loadObjectFile("lc3os.obj");
    state.pc = RESET_PC;
}

void lc3::core::Simulator::simulate(void)
{
    state.running = true;
    collecting_input = true;
    inputter.beginInput();
    std::thread input_thread(&core::Simulator::handleInput, this);

    while(state.running && (state.mem[MCR].getValue() & 0x8000) != 0) {
        if(!state.hit_breakpoint) {
            executeEvent(CallbackEvent(state.pre_instruction_callback_v, state.pre_instruction_callback));
            if(state.hit_breakpoint) {
                break;
            }
        }
        state.hit_breakpoint = false;

        std::vector<IEvent const *> events = executeInstruction();
        events.push_back(new CallbackEvent(state.post_instruction_callback_v, state.post_instruction_callback));
        executeEventChain(events);
        updateDevices();
        events = checkAndSetupInterrupts();
        executeEventChain(events);
    }

    state.running = false;
    collecting_input = false;
    input_thread.join();
    inputter.endInput();
}

std::vector<lc3::core::IEvent const *> lc3::core::Simulator::executeInstruction(void)
{
    uint32_t encoded_inst = state.mem[state.pc].getValue();

    IInstruction * candidate;
    if(! decoder.findInstructionByEncoding(encoded_inst, candidate)) {
        logger.printf(lc3::utils::PrintType::PRINT_TYPE_ERROR, true, "invalid instruction 0x%0.4x", encoded_inst);
        throw utils::exception("invalid instruction");
    }

    candidate->assignOperands(encoded_inst);
    logger.printf(lc3::utils::PrintType::PRINT_TYPE_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc,
        state.mem[state.pc].getLine().c_str(), encoded_inst);
    state.pc += 1;
    std::vector<IEvent const *> events = candidate->execute(state);
    delete candidate;

    return events;
}

void lc3::core::Simulator::updateDevices(void)
{
    uint16_t value = state.mem[DSR].getValue();
    state.mem[DSR].setValue(value | 0x8000);
}

std::vector<lc3::core::IEvent const *> lc3::core::Simulator::checkAndSetupInterrupts(void)
{
    std::vector<IEvent const *> ret;

    uint32_t value = state.mem[KBSR].getValue();
    if((value & 0xC000) == 0xC000) {
        ret.push_back(new RegEvent(6, state.regs[6] - 1));
        ret.push_back(new MemWriteEvent(state.regs[6] - 1, state.psr));
        ret.push_back(new RegEvent(6, state.regs[6] - 2));
        ret.push_back(new MemWriteEvent(state.regs[6] - 2, state.pc));
        ret.push_back(new PSREvent((state.psr & 0x78FF) | 0x0400));
        ret.push_back(new PCEvent(state.mem[0x0180].getValue()));
        ret.push_back(new MemWriteEvent(KBSR, state.mem[KBSR].getValue() & 0x7fff));
        ret.push_back(new CallbackEvent(state.interrupt_enter_callback_v, state.interrupt_enter_callback));
    }

    return ret;
}

void lc3::core::Simulator::executeEventChain(std::vector<core::IEvent const *> & events)
{
    for(uint32_t i = 0; i < events.size(); i += 1) {
        executeEvent(*events[i]);
        delete events[i];
    }

    events.clear();
}

void lc3::core::Simulator::executeEvent(core::IEvent const & event)
{
    logger.printf(lc3::utils::PrintType::PRINT_TYPE_EXTRA, false, "  %s", event.getOutputString(state).c_str());
    event.updateState(state);
}

void lc3::core::Simulator::reset(void)
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

    state.pre_instruction_callback_v = false;
    state.post_instruction_callback_v = false;
    state.interrupt_enter_callback_v = false;
    state.interrupt_exit_callback_v = false;
}

void lc3::core::Simulator::registerPreInstructionCallback(callback_func_t func)
{
    state.pre_instruction_callback_v = true;
    state.pre_instruction_callback = func;
}

void lc3::core::Simulator::registerPostInstructionCallback(callback_func_t func)
{
    state.post_instruction_callback_v = true;
    state.post_instruction_callback = func;
}

void lc3::core::Simulator::registerInterruptEnterCallback(callback_func_t func)
{
    state.interrupt_enter_callback_v = true;
    state.interrupt_enter_callback = func;
}

void lc3::core::Simulator::registerInterruptExitCallback(callback_func_t func)
{
    state.interrupt_exit_callback_v = true;
    state.interrupt_exit_callback = func;
}

void lc3::core::Simulator::registerSubEnterCallback(callback_func_t func)
{
    state.sub_enter_callback_v = true;
    state.sub_enter_callback = func;
}

void lc3::core::Simulator::registerSubExitCallback(callback_func_t func)
{
    state.sub_exit_callback_v = true;
    state.sub_exit_callback = func;
}

void lc3::core::Simulator::handleInput(void)
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
