/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cassert>
#include <chrono>
#include <fstream>
#include <string>
#include <random>

#include "device_regs.h"
#include "interface.h"
#include "lc3os.h"

lc3::sim::sim(lc3::utils::IPrinter & printer, lc3::utils::IInputter & inputter, uint32_t print_level) :
    printer(printer), inputter(inputter), simulator(printer, inputter, print_level)
{
    loadOS();

    auto callback_dispatcher = std::bind(callbackDispatcher, this, std::placeholders::_1, std::placeholders::_2);
    simulator.registerCallback(core::CallbackType::PRE_INST, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::POST_INST, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::SUB_ENTER, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::SUB_EXIT, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::EX_ENTER, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::EX_EXIT, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::INT_ENTER, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::INT_EXIT, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::BREAKPOINT, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::INPUT_REQUEST, callback_dispatcher);
    simulator.registerCallback(core::CallbackType::INPUT_POLL, callback_dispatcher);

    total_inst_exec = 0;
    cur_inst_exec_limit = 0;
    target_inst_exec = 0;
    cur_sub_depth = 0;
}

bool lc3::sim::loadObjFile(std::string const & filename)
{
    std::ifstream obj_file(filename, std::ios_base::binary);
    if(! obj_file) {
        printer.print("could not open file " + filename);
        printer.newline();
        return false;
    }

    try {
        simulator.loadObj(filename, obj_file);
    } catch(utils::exception const & e) {
        printer.print("caught exception: " + std::string(e.what()));
        printer.newline();
        return false;
    }

    return true;
}

void lc3::sim::setup(void)
{
    simulator.setup();
}

void lc3::sim::zeroState(void)
{
    simulator.reinitialize();
    loadOS();
}

uint64_t lc3::sim::randomizeState(uint64_t seed)
{
    std::random_device dev;
    if(seed == 0) {
        seed = dev();
    }
    std::mt19937 gen((unsigned int) seed);
    std::uniform_int_distribution<> dis(0x0000, 0xffff);

    simulator.reinitialize();

    core::MachineState & state = simulator.getMachineState();

    for(uint32_t i = 0; i < USER_END; ++i) {
        state.writeMem(i, dis(gen));
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        state.writeReg(i, dis(gen));
    }

    state.writeSSP(dis(gen));

    loadOS();

    return seed;
}

void lc3::sim::setRunInstLimit(uint64_t inst_limit) { cur_inst_exec_limit = inst_limit; }

bool lc3::sim::run(void)
{
    run_type = RunType::NORMAL;
    return runHelper();
}

bool lc3::sim::runUntilHalt(void)
{
    run_type = RunType::UNTIL_HALT;
    return runHelper();
}

bool lc3::sim::runUntilInputRequested(void)
{
    run_type = RunType::UNTIL_INPUT_REQUESTED;
    return runHelper();
}

void lc3::sim::asyncInterrupt(void)
{
    simulator.asyncInterrupt();
}

bool lc3::sim::stepIn(void)
{
    run_type = RunType::NORMAL;
    setRunInstLimit(1);
    return runHelper();
}

bool lc3::sim::stepOver(void)
{
    run_type = RunType::UNTIL_DEPTH;
    cur_sub_depth = 0;
    setRunInstLimit(0);
    return runHelper();
}

bool lc3::sim::stepOut(void)
{
    run_type = RunType::UNTIL_DEPTH;
    cur_sub_depth = 1;
    setRunInstLimit(0);
    return runHelper();
}

lc3::core::MachineState & lc3::sim::getMachineState(void) { return simulator.getMachineState(); }
lc3::core::MachineState const & lc3::sim::getMachineState(void) const { return simulator.getMachineState(); }

uint16_t lc3::sim::readReg(uint16_t id) const { return simulator.getMachineState().readReg(id); }
uint16_t lc3::sim::readMem(uint16_t addr) const { return simulator.getMachineState().readMem(addr).first; }
std::string lc3::sim::getMemLine(uint16_t addr) const { return simulator.getMachineState().getMemLine(addr); }
uint16_t lc3::sim::readPC(void) const { return simulator.getMachineState().readPC(); }
uint16_t lc3::sim::readPSR(void) const { return simulator.getMachineState().readPSR(); }
uint16_t lc3::sim::readMCR(void) const { return simulator.getMachineState().readMCR(); }
char lc3::sim::readCC(void) const { return utils::getBits(readPSR(), 2, 0); }
void lc3::sim::writeReg(uint16_t id, uint16_t value) { simulator.getMachineState().writeReg(id, value); }
void lc3::sim::writeMem(uint16_t addr, uint16_t value) { simulator.getMachineState().writeMem(addr, value); }
void lc3::sim::writeStringMem(uint16_t addr, std::string const & value)
{
    core::MachineState & state = simulator.getMachineState();

    state.setMemLine(addr, value);
    for(uint32_t i = 0; i < value.size(); i += 1) {
        state.writeMem(addr + i, static_cast<uint16_t>(value[i]));
        if(i != 0) {
            state.setMemLine(addr + i, std::string(1, value[i]));
        }
    }
    state.writeMem(addr + static_cast<uint16_t>(value.size()), 0);
    state.setMemLine(addr + static_cast<uint16_t>(value.size()), "");
}
void lc3::sim::setMemLine(uint16_t addr, std::string const & value)
{ simulator.getMachineState().setMemLine(addr, value); }
void lc3::sim::writePC(uint16_t value) { simulator.getMachineState().writePC(value); }
void lc3::sim::writePSR(uint16_t value) { simulator.getMachineState().writePSR(value); }
void lc3::sim::writeMCR(uint16_t value) { simulator.getMachineState().writeMCR(value); }
void lc3::sim::writeCC(char value)
{
    uint16_t bits = (value == 'P' ? 0x1 : 0);
    bits |= (value == 'Z' ? 0x2 : 0);
    bits |= (value == 'N' ? 0x4 : 0);
    simulator.getMachineState().writePSR((readPSR() & 0x7FF8) | bits);
}

void lc3::sim::setBreakpoint(uint16_t addr) { simulator.addBreakpoint(addr); }
void lc3::sim::removeBreakpoint(uint16_t addr) { simulator.removeBreakpoint(addr); }

bool lc3::sim::didExceedInstLimit(void) const { return total_inst_exec == target_inst_exec; }

void lc3::sim::registerCallback(lc3::core::CallbackType type, lc3::sim::Callback func) { callbacks[type] = func; }

lc3::utils::IPrinter & lc3::sim::getPrinter(void) { return printer; }
lc3::utils::IPrinter const & lc3::sim::getPrinter(void) const { return printer; }
lc3::utils::IInputter & lc3::sim::getInputter(void) { return inputter; }
lc3::utils::IInputter const & lc3::sim::getInputter(void) const { return inputter; }
void lc3::sim::setPrintLevel(uint32_t print_level) { simulator.setPrintLevel(print_level); }
void lc3::sim::setIgnorePrivilege(bool ignore_privilege) { simulator.setIgnorePrivilege(ignore_privilege); }

uint64_t lc3::sim::getInstExecCount(void) const { return total_inst_exec; }

void lc3::sim::loadOS(void)
{
    core::Assembler assembler(printer, 0, false);
    assembler.setFilename("lc3os");

    std::stringstream src_buffer;
    src_buffer << lc3::core::getOSSrc();
    std::shared_ptr<std::stringstream> obj_stream = assembler.assemble(src_buffer);
    simulator.loadObj("lc3os", *obj_stream);
}

bool lc3::sim::runHelper(void)
{
    encountered_lc3_exception = false;
    target_inst_exec = total_inst_exec + cur_inst_exec_limit;

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    try {
        simulator.simulate();
    } catch(utils::exception const & e) {
        printer.print("caught exception: " + std::string(e.what()));
        printer.newline();
        return false;
    }

#ifdef _ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    printer.newline();
    printer.print("elapsed time: " + std::to_string(elapsed.count() * 1000) + " ms");
    printer.newline();
#endif

    return ! encountered_lc3_exception;
}

void lc3::sim::callbackDispatcher(lc3::sim * sim_inst, lc3::core::CallbackType type, lc3::core::MachineState & state)
{
    using namespace lc3::core;

    if(type == CallbackType::PRE_INST) {
        if(sim_inst->run_type == RunType::UNTIL_HALT && state.readMem(state.readPC()).first == 0xf025) {
            // Halt if current instruction is HALT.
            sim_inst->simulator.triggerSuspend();
        }
    } else if(type == CallbackType::POST_INST) {
        // Increment total instruction count
        ++(sim_inst->total_inst_exec);
        if(sim_inst->cur_inst_exec_limit != 0) {
            if(sim_inst->total_inst_exec == sim_inst->target_inst_exec) {
                // If an instruction limit is set (i.e. cur_inst_exec_limit != 0), halt when target is reached.
                sim_inst->simulator.triggerSuspend();
            }
        }

        if(sim_inst->run_type == RunType::UNTIL_DEPTH) {
            if(sim_inst->cur_sub_depth == 0) {
                sim_inst->simulator.triggerSuspend();
            }
        }
    } else if(type == CallbackType::SUB_ENTER) {
        ++(sim_inst->cur_sub_depth);
    } else if(type == CallbackType::SUB_EXIT) {
        if(sim_inst->cur_sub_depth > 0) {
            --(sim_inst->cur_sub_depth);
        }
    } else if(type == CallbackType::EX_ENTER) {
        // Mark that execution resulted in LC-3 exception.
        sim_inst->encountered_lc3_exception = true;
        ++(sim_inst->cur_sub_depth);
    } else if(type == CallbackType::EX_EXIT) {
        if(sim_inst->cur_sub_depth > 0) {
            --(sim_inst->cur_sub_depth);
        }
    } else if(type == CallbackType::INT_ENTER) {
        ++(sim_inst->cur_sub_depth);
    } else if(type == CallbackType::INT_EXIT) {
        if(sim_inst->cur_sub_depth > 0) {
            --(sim_inst->cur_sub_depth);
        }
    } else if(type == CallbackType::INPUT_REQUEST) {
        if(sim_inst->run_type == RunType::UNTIL_INPUT_REQUESTED) {
            sim_inst->simulator.triggerSuspend();
        }
    }

    auto search = sim_inst->callbacks.find(type);
    if(search != sim_inst->callbacks.end() && search->second != nullptr) {
        search->second(type, *sim_inst);
    }
}

lc3::as::as(utils::IPrinter & printer, uint32_t print_level, bool enable_liberal_asm) :
    printer(printer), assembler(printer, print_level, enable_liberal_asm)
{ }

lc3::optional<std::string> lc3::as::assemble(std::string const & asm_filename)
{
    std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
    assembler.setFilename(asm_filename);
    std::ifstream in_file(asm_filename);
    if(! in_file.is_open()) {
        printer.print("could not open file " + asm_filename);
        printer.newline();
        return {};
    }

    printer.print("attempting to assemble " + asm_filename + " into " + obj_filename);
    printer.newline();

    std::shared_ptr<std::stringstream> out_stream;

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    try {
        out_stream = assembler.assemble(in_file);
    } catch(utils::exception const & e) {
        printer.print("caught exception: " + std::string(e.what()));
        printer.newline();
        return {};
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
        return {};
    }

    out_file << out_stream->rdbuf();
    out_file.close();

    return obj_filename;
}

lc3::conv::conv(utils::IPrinter & printer, uint32_t print_level) :
    printer(printer), converter(printer, print_level)
{ }

lc3::optional<std::string> lc3::conv::convertBin(std::string const & bin_filename)
{
    std::string obj_filename(bin_filename.substr(0, bin_filename.find_last_of('.')) + ".obj");
    std::ifstream in_file(bin_filename);
    if(! in_file.is_open()) {
        printer.print("could not open file " + bin_filename);
        printer.newline();
        return {};
    }

    printer.print("attempting to convert " + bin_filename + " into " + obj_filename);
    printer.newline();

    std::shared_ptr<std::stringstream> out_stream;

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    try {
        out_stream = converter.convertBin(in_file);
    } catch(utils::exception const & e) {
        printer.print("caught exception: " + std::string(e.what()));
        printer.newline();
        return {};
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
        return {};
    }

    out_file << out_stream->rdbuf();
    out_file.close();

    return obj_filename;
}

void lc3::as::setEnableLiberalAsm(bool enable) { assembler.setLiberalAsm(enable); }
