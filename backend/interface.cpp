/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cassert>
#include <chrono>
#include <string>
#include <random>

#include "device_regs.h"
#include "interface.h"
#include "lc3os.h"

lc3::sim::sim(utils::IPrinter & printer, utils::IInputter & inputter, bool threaded_input, uint32_t print_level,
    bool propagate_exceptions) :
    printer(printer), simulator(*this, printer, inputter, print_level, threaded_input),
    propagate_exceptions(propagate_exceptions)
{
    simulator.registerPreInstructionCallback(lc3::sim::preInstructionCallback);
    simulator.registerPostInstructionCallback(lc3::sim::postInstructionCallback);
    simulator.registerInterruptEnterCallback(lc3::sim::interruptEnterCallback);
    simulator.registerInterruptExitCallback(lc3::sim::interruptExitCallback);
    simulator.registerSubEnterCallback(lc3::sim::subEnterCallback);
    simulator.registerSubExitCallback(lc3::sim::subExitCallback);
    simulator.registerWaitForInputCallback(lc3::sim::waitForInputCallback);
    if(propagate_exceptions) {
        loadOS();
    } else {
        try {
            loadOS();
        } catch(utils::exception const & e) {
            (void) e;
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()));
			printer.newline();
#endif
        }
    }
    restart();
    run_type = RunType::NORMAL;
}

bool lc3::sim::loadObjFile(std::string const & obj_filename)
{
    std::ifstream obj_file(obj_filename, std::ios_base::binary);
    if(! obj_file) {
        printer.print("could not open file " + obj_filename);
		printer.newline();
        if(propagate_exceptions) {
            throw utils::exception("could not open file");
        } else {
            return false;
        }
    }

    if(propagate_exceptions) {
        simulator.loadObj(obj_file);
    } else {
        try {
            simulator.loadObj(obj_file);
        } catch(utils::exception const & e) {
            (void) e;
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()));
			printer.newline();
#endif
            return false;
        }
    }
    return true;
}

void lc3::sim::reinitialize(void)
{
    simulator.reinitialize();
    loadOS();
}

void lc3::sim::loadOS(void)
{
    core::Assembler assembler(printer, simulator.getPrintLevel());
    assembler.setFilename("lc3os");

    std::stringstream src_buffer;
    src_buffer << lc3::core::getOSSrc();
    std::stringstream obj_stream = assembler.assemble(src_buffer);
    simulator.loadObj(obj_stream);
    getMachineState().pc = RESET_PC;
}

void lc3::sim::randomize(void)
{
    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> dis(0x0000, 0xffff);

    for(uint32_t i = 0x3000; i < 0xfe00; i += 1) {
        setMem(i, dis(gen));
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        setReg(i, dis(gen));
    }

    restart();
}

void lc3::sim::restart(void)
{
    uint32_t mcr = getMem(MCR);
    setMem(MCR, mcr | 0x8000);
}

void lc3::sim::setRunInstLimit(uint64_t inst_limit)
{
    this->inst_limit = inst_limit;
}

bool lc3::sim::run(void)
{
    return run(RunType::NORMAL);
}

bool lc3::sim::runUntilHalt(void)
{
    return run(RunType::UNTIL_HALT);
}

bool lc3::sim::runUntilInputPoll(void)
{
    return run(RunType::UNTIL_INPUT);
}

bool lc3::sim::stepIn(void)
{
    setRunInstLimit(1);
    return run(RunType::NORMAL);
}

bool lc3::sim::stepOver(void)
{
    sub_depth = 0;
    setRunInstLimit(0);
    return run(RunType::UNTIL_DEPTH);
}

bool lc3::sim::stepOut(void)
{
    sub_depth = 1;
    setRunInstLimit(0);
    return run(RunType::UNTIL_DEPTH);
}


bool lc3::sim::run(lc3::sim::RunType cur_run_type)
{
    restart();
    run_type = cur_run_type;
    total_inst_limit += inst_limit;
    remaining_inst_count = inst_limit;

    if(propagate_exceptions) {
        simulator.simulate();
    } else {
        try {
            simulator.simulate();
        } catch(utils::exception const & e) {
            (void) e;
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()));
			printer.newline();
#endif
            return false;
        }
    }
    return true;
}

void lc3::sim::pause(void)
{
    simulator.disableClock();
}

lc3::core::MachineState & lc3::sim::getMachineState(void) { return simulator.getMachineState(); }

lc3::core::MachineState const & lc3::sim::getMachineState(void) const { return simulator.getMachineState(); }

uint64_t lc3::sim::getInstExecCount(void) const { return inst_exec_count; }

bool lc3::sim::didExceedInstLimit(void) const { return inst_exec_count >= total_inst_limit; }

std::vector<lc3::Breakpoint> const & lc3::sim::getBreakpoints(void) const { return breakpoints; }

uint16_t lc3::sim::getReg(uint16_t id) const
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
#else
    id &= 0x7;
#endif
    return getMachineState().regs[id];
}

uint16_t lc3::sim::getMem(uint16_t addr) const
{
    return getMachineState().readMemRaw(addr);
}

std::string lc3::sim::getMemLine(uint16_t addr) const
{
    return getMachineState().mem[addr].getLine();
}

uint16_t lc3::sim::getPC(void) const { return getMachineState().pc; }

uint16_t lc3::sim::getPSR(void) const { return getMem(PSR); }

uint16_t lc3::sim::getMCR(void) const { return getMem(MCR); }

char lc3::sim::getCC(void) const
{
    uint32_t value = getPSR() & 0x7;
#ifdef _ENABLE_DEBUG
    assert(value == 0x4 || value == 0x2 || value == 0x1);
#else
    if(value == 0x4 || value == 0x2 || value == 0x1) {
        value = 0x1;
    }
#endif
    if(value == 0x4) { return 'N'; }
    else if(value == 0x2) { return 'Z'; }
    else { return 'P'; }
}

void lc3::sim::setReg(uint16_t id, uint16_t value)
{
    getMachineState().regs[id] = value;
}

void lc3::sim::setMem(uint16_t addr, uint16_t value)
{
    getMachineState().writeMemSafe(addr, value);
    getMachineState().mem[addr].setLine("");
}

void lc3::sim::setMemString(uint16_t addr, std::string const & value)
{
    for(uint32_t i = 0; i < value.size(); i += 1) {
        getMachineState().writeMemRaw(addr + i, static_cast<uint32_t>(value[i]));
        getMachineState().mem[addr + i].setLine(std::string(1, value[i]));
    }
    getMachineState().writeMemRaw((uint32_t) (addr + value.size()), 0);
    getMachineState().mem[addr + value.size()].setLine(value);
}

void lc3::sim::setMemLine(uint16_t addr, std::string const & value)
{
    getMachineState().mem[addr].setLine(value);
}

void lc3::sim::setPC(uint16_t value)
{
#ifdef _ENABLE_DEBUG
    assert(value <= 0xfe00u);
#else
    value = std::min<uint16_t>(value, 0xfe00);
#endif
    getMachineState().pc = value;
    while(! getMachineState().sys_call_types.empty()) {
        getMachineState().sys_call_types.pop();
    }
    sub_depth = 0;

    // re-enable system
    restart();
}

void lc3::sim::setPSR(uint16_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8707) & 0xffff)) == 0x0000);
#endif
    setMem(PSR, value);
}

void lc3::sim::setMCR(uint16_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8000) & 0xffff)) == 0x0000);
#endif
    setMem(MCR, value);
}

void lc3::sim::setCC(char value)
{
    value |= 0x20;
#ifdef _ENABLE_DEBUG
    assert(value == 'n' || value == 'z' || value == 'p');
#else
    if(! (value == 'n' || value == 'z' || value == 'p')) { return; }
#endif
    uint32_t new_value = 0;
    if(value == 'n') { new_value = 0x4; }
    else if(value == 'z') { new_value = 0x2; }
    else { new_value = 0x1; }
    uint32_t psr = getPSR();
    setPSR((psr & 0xfff8) | new_value);
}

lc3::Breakpoint lc3::sim::setBreakpoint(uint16_t addr)
{
    Breakpoint bp(breakpoint_id, addr, this);
    breakpoints.push_back(bp);
    breakpoint_id += 1;
    return bp;
}

bool lc3::sim::removeBreakpointByID(uint32_t id)
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

bool lc3::sim::removeBreakpointByAddr(uint16_t addr)
{
    auto it = breakpoints.begin();
    bool found = false;
    for(; it != breakpoints.end(); ++it) {
        if(it->loc == addr) {
            found = true;
            break;
        }
    }

    if(found) {
        breakpoints.erase(it);
    }

    return found;
}

void lc3::sim::registerPreInstructionCallback(callback_func_t func)
{
    pre_instruction_callback_v = true;
    pre_instruction_callback = func;
}

void lc3::sim::registerPostInstructionCallback(callback_func_t func)
{
    post_instruction_callback_v = true;
    post_instruction_callback = func;
}

void lc3::sim::registerInterruptEnterCallback(callback_func_t func)
{
    interrupt_enter_callback_v = true;
    interrupt_enter_callback = func;
}

void lc3::sim::registerInterruptExitCallback(callback_func_t func)
{
    interrupt_exit_callback_v = true;
    interrupt_exit_callback = func;
}

void lc3::sim::registerSubEnterCallback(callback_func_t func)
{
    sub_enter_callback_v = true;
    sub_enter_callback = func;
}


void lc3::sim::registerSubExitCallback(callback_func_t func)
{
    sub_exit_callback_v = true;
    sub_exit_callback = func;
}

void lc3::sim::registerWaitForInputCallback(callback_func_t func)
{
    wait_for_input_callback_v = true;
    wait_for_input_callback = func;
}

void lc3::sim::registerBreakpointCallback(breakpoint_callback_func_t func)
{
    breakpoint_callback_v = true;
    breakpoint_callback = func;
}

lc3::utils::IPrinter & lc3::sim::getPrinter(void) { return printer; }
lc3::utils::IPrinter const & lc3::sim::getPrinter(void) const { return printer; }
void lc3::sim::setPrintLevel(uint32_t print_level) { simulator.setPrintLevel(print_level); }
void lc3::sim::setPropagateExceptions(void) { propagate_exceptions = true; }
void lc3::sim::clearPropagateExceptions(void) { propagate_exceptions = false; }
void lc3::sim::setIgnorePrivilege(bool ignore) { simulator.setIgnorePrivilege(ignore); }

void lc3::sim::preInstructionCallback(lc3::sim & sim_inst, lc3::core::MachineState & state)
{
    if(sim_inst.run_type == RunType::UNTIL_HALT && state.readMemRaw(state.pc) == 0xf025) {
        sim_inst.pause();
    }

    if(sim_inst.pre_instruction_callback_v) {
        sim_inst.pre_instruction_callback(state);
    }
}

void lc3::sim::postInstructionCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.inst_exec_count += 1;

    if(sim_inst.remaining_inst_count >= 0) {
        sim_inst.remaining_inst_count -= 1;
        if(sim_inst.remaining_inst_count == 0) {
            sim_inst.pause();
        }
    }

    if(sim_inst.run_type == RunType::UNTIL_DEPTH && sim_inst.sub_depth <= 0) {
        sim_inst.pause();
    }

    for(auto const & x : sim_inst.breakpoints) {
        if(state.pc == x.loc) {
            if(sim_inst.breakpoint_callback_v) {
                sim_inst.breakpoint_callback(state, x);
            }
            sim_inst.pause();
            break;
        }
    }

    if(sim_inst.post_instruction_callback_v) {
        sim_inst.post_instruction_callback(state);
    }
}

void lc3::sim::interruptEnterCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth += 1;

    if(sim_inst.interrupt_enter_callback_v) {
        sim_inst.interrupt_enter_callback(state);
    }
}

void lc3::sim::interruptExitCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth -= 1;

    if(sim_inst.interrupt_exit_callback_v) {
        sim_inst.interrupt_exit_callback(state);
    }
}

void lc3::sim::subEnterCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth += 1;

    if(sim_inst.sub_enter_callback_v) {
        sim_inst.sub_enter_callback(state);
    }
}

void lc3::sim::subExitCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth -= 1;

    if(sim_inst.sub_exit_callback_v) {
        sim_inst.sub_exit_callback(state);
    }
}

void lc3::sim::waitForInputCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    if(sim_inst.run_type == RunType::UNTIL_INPUT) {
        sim_inst.pause();
    }

    if(sim_inst.wait_for_input_callback_v) {
        sim_inst.wait_for_input_callback(state);
    }
}

lc3::optional<std::string> lc3::as::assemble(std::string const & asm_filename)
{
    std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
    assembler.setFilename(asm_filename);
    std::ifstream in_file(asm_filename);
    if(! in_file.is_open()) {
        printer.print("could not open file " + asm_filename);
		printer.newline();
        if(propagate_exceptions) {
            throw lc3::utils::exception("could not open file for reading");
        } else {
            return {};
        }
    }

    printer.print("attemping to assemble " + asm_filename + " into " + obj_filename);
	printer.newline();

    std::stringstream out_stream;

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    if(propagate_exceptions) {
        out_stream = assembler.assemble(in_file);
    } else {
        try {
            out_stream = assembler.assemble(in_file);
        } catch(utils::exception const & e) {
            (void) e;
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()));
			printer.newline();
#endif
            return {};
        }
    }

#ifdef _ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    printer.print("elapsed time: " + std::to_string(elapsed.count() * 1000) + " ms");
    printer.newline();
#endif

    printer.print("assembly successful");
	printer.newline();
    in_file.close();

    std::ofstream out_file(obj_filename, std::ios_base::binary);
    if(! out_file.is_open()) {
        printer.print("could not open " + obj_filename + " for writing");
		printer.newline();
        if(propagate_exceptions) {
            throw lc3::utils::exception("could not open file for writing");
        } else {
            return {};
        }
    }

    out_file << out_stream.rdbuf();
    out_file.close();

    return obj_filename;
}

lc3::optional<std::string> lc3::conv::convertBin(std::string const & bin_filename)
{
    std::string obj_filename(bin_filename.substr(0, bin_filename.find_last_of('.')) + ".obj");
    std::ifstream in_file(bin_filename);
    if(! in_file.is_open()) {
        printer.print("could not open file " + bin_filename);
		printer.newline();
        if(propagate_exceptions) {
            throw lc3::utils::exception("could not open file for reading");
        } else {
            return {};
        }
    }

    printer.print("attemping to convert " + bin_filename + " into " + obj_filename);
	printer.newline();

    std::stringstream out_stream;

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    if(propagate_exceptions) {
        out_stream = converter.convertBin(in_file);
    } else {
        try {
            out_stream = converter.convertBin(in_file);
        } catch(utils::exception const & e) {
            (void) e;
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()));
			printer.newline();
#endif
            return {};
        }
    }

#ifdef _ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    printer.print("elapsed time: " + std::to_string(elapsed.count() * 1000) + " ms");
    printer.newline();
#endif

    printer.print("conversion successful");
	printer.newline();
    in_file.close();

    std::ofstream out_file(obj_filename, std::ios_base::binary);
    if(! out_file.is_open()) {
        printer.print("could not open " + obj_filename + " for writing");
		printer.newline();
        if(propagate_exceptions) {
            throw lc3::utils::exception("could not open file for writing");
        } else {
            return {};
        }
    }

    out_file << out_stream.rdbuf();
    out_file.close();

    return obj_filename;
}

void lc3::as::setPropagateExceptions(void) { propagate_exceptions = true; }
void lc3::as::clearPropagateExceptions(void) { propagate_exceptions = false; }
