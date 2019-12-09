/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <fstream>
#include <mutex>
#include <thread>
#include <sstream>

#include "device_regs.h"
#include "lc3os.h"
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
    uint32_t print_level, bool threaded_input) : state(simulator, logger), logger(printer, print_level),
    inputter(inputter), threaded_input(threaded_input), collecting_input(false)
{
    state.mem.resize(1 << 16);
    reinitialize();

    state.pre_instruction_callback_v = false;
    state.post_instruction_callback_v = false;
    state.interrupt_enter_callback_v = false;
    state.interrupt_exit_callback_v = false;
}

void Simulator::loadObj(std::istream & buffer)
{
    using namespace lc3::utils;

    uint32_t fill_pc = 0;
    uint32_t offset = 0;
    bool first_orig_set = false;

    // Verify header.
    std::string expected_header = getMagicHeader();
    char * header = new char[expected_header.size()];
    if(buffer.read(header, expected_header.size())) {
        for(uint32_t i = 0; i < expected_header.size(); i += 1) {
            if(header[i] != expected_header[i]) {
                logger.printf(PrintType::P_ERROR, true, "invalid header (is this a .obj file?); try re-assembling");
                throw utils::exception("invalid header (is this a .obj file?); try re-assembling");
            }
        }
    } else {
        logger.printf(PrintType::P_ERROR, true, "could not read header");
        throw utils::exception("could not read header");
    }
    delete[] header;

    // Verify version number matches current version number.
    std::string expected_version = getVersionString();
    char * version = new char[expected_version.size()];
    if(buffer.read(version, expected_version.size())) {
        for(uint32_t i = 0; i < expected_version.size(); i += 1) {
            if(version[i] != expected_version[i]) {
                logger.printf(PrintType::P_ERROR, true, "mismatched version numbers; try re-assembling");
                throw utils::exception("mismatched version numbers; try re-assembling");
            }
        }
    } else {
        logger.printf(PrintType::P_ERROR, true, "could not version number; try re-assembling");
        throw utils::exception("could not read version number; try re-assembling");
    }
    delete[] version;

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
    enableClock();
}

void Simulator::simulate(void)
{
    std::thread input_thread;
    utils::exception exception;
    bool exception_valid = false;

    try {
        enableClock();

        collecting_input = true;
        inputter.beginInput();
        if(threaded_input) {
            input_thread = std::thread(&core::Simulator::inputThread, this);
        }

        while(isClockEnabled()) {
            executeEvent(std::make_shared<CallbackEvent>(state.pre_instruction_callback_v,
                state.pre_instruction_callback));
            if(! isClockEnabled()) { break; }    // pre_instruction_callback may pause machine
            std::vector<PIEvent> events = executeInstruction();
            events.push_back(std::make_shared<CallbackEvent>(state.post_instruction_callback_v,
                state.post_instruction_callback));
            executeEventChain(events);
            updateDevices();
            if(! threaded_input) {
                collectInput();
            }
            checkAndSetupInterrupts();
        }
    } catch(utils::exception & e) {
        exception = e;
        exception_valid = true;
    } catch(std::exception & e) { (void) e; }

    disableClock();
    collecting_input = false;
    if(threaded_input && input_thread.joinable()) {
        input_thread.join();
    }
    inputter.endInput();

    if(exception_valid) {
        throw exception;
    }
}

void Simulator::enableClock(void)
{
    uint16_t mcr = state.readMemRaw(MCR);
    state.writeMemRaw(MCR, mcr | 0x8000);
}

void Simulator::disableClock(void)
{
    uint16_t mcr = state.readMemRaw(MCR);
    state.writeMemRaw(MCR, mcr & (~0x8000));
}

bool Simulator::isClockEnabled(void) const
{
    return (state.readMemRaw(MCR) & 0x8000) == 0x8000;
}

std::vector<PIEvent> Simulator::executeInstruction(void)
{
    if(! state.ignore_privilege && ((state.pc <= SYSTEM_END || state.pc >= MMIO_START)
        && (state.readMemRaw(PSR) & 0x8000) == 0x8000))
    {
        logger.printf(lc3::utils::PrintType::P_EXTRA, true, "illegal PC 0x%0.4x accessed", state.pc);
        return IInstruction::buildSysCallEnterHelper(state, INTEX_TABLE_START + 0x0, MachineState::SysCallType::EX);
    }
    uint32_t encoded_inst = state.readMemSafe(state.pc);

    optional<PIInstruction> candidate = decoder.findInstructionByEncoding(encoded_inst);
    if(! candidate) {
        logger.printf(lc3::utils::PrintType::P_EXTRA, true, "illegal opcode");
        return IInstruction::buildSysCallEnterHelper(state, INTEX_TABLE_START + 0x1, MachineState::SysCallType::EX);
    }

    (*candidate)->assignOperands(encoded_inst);
    logger.printf(lc3::utils::PrintType::P_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc,
        state.mem[state.pc].getLine().c_str(), encoded_inst);
    state.pc = (state.pc + 1) & 0xffff;
    std::vector<PIEvent> events = (*candidate)->execute(state);

    return events;
}

void Simulator::updateDevices(void)
{
    uint16_t value = state.readMemRaw(DSR);
    state.writeMemRaw(DSR, value | 0x8000);
}

void Simulator::checkAndSetupInterrupts(void)
{
    uint32_t value = state.readMemRaw(KBSR);

    if(((value & 0xc000) == 0xc000) && ((state.readMemRaw(PSR) & 0x0700) == 0)) {
        logger.printf(lc3::utils::PrintType::P_EXTRA, true, "jumping to keyboard ISR");

        std::vector<PIEvent> events = IInstruction::buildSysCallEnterHelper(state, INTEX_TABLE_START + 0x80,
            MachineState::SysCallType::INT, [](uint32_t psr_value) { return (psr_value & 0x78ff) | 0x0400; });
        // events.push_back(std::make_shared<MemWriteEvent>(KBSR, state.readMemRaw(KBSR) & 0x7fff));
        events.push_back(std::make_shared<CallbackEvent>(state.post_instruction_callback_v,
                state.post_instruction_callback));

        executeEventChain(events);
    }
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
    enableClock();
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

void Simulator::registerExceptionEnterCallback(callback_func_t func)
{
    state.exception_enter_callback_v = true;
    state.exception_enter_callback = func;
}

void Simulator::registerExceptionExitCallback(callback_func_t func)
{
    state.exception_exit_callback_v = true;
    state.exception_exit_callback = func;
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

void Simulator::registerWaitForInputCallback(callback_func_t func)
{
    state.wait_for_input_callback_v = true;
    state.wait_for_input_callback = func;
}

void lc3::core::Simulator::setIgnorePrivilege(bool ignore)
{
    state.ignore_privilege = ignore;
}

void Simulator::collectInput(void)
{
    char c;
    uint16_t kbsr = state.readMemRaw(KBSR);
    if((kbsr & 0x8000) == 0 && inputter.getChar(c)) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        state.writeMemRaw(KBSR, kbsr | 0x8000);
        state.writeMemRaw(KBDR, ((uint32_t) c) & 0xFF);
    }
}

void Simulator::inputThread(void)
{
    while(collecting_input) {
        collectInput();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
