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

void lc3::sim::zeroState(void)
{
    core::MachineState & state = simulator.getMachineState();

    for(uint32_t i = 0; i <= USER_END; ++i) {
        state.writeMem(i, 0);
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        state.writeReg(i, 0);
    }

    loadOS();
}

void lc3::sim::randomizeState(void)
{
    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> dis(0x0000, 0xffff);

    core::MachineState & state = simulator.getMachineState();

    for(uint32_t i = 0; i < USER_END; ++i) {
        state.writeMem(i, dis(gen));
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        state.writeReg(i, dis(gen));
    }

    loadOS();
}

bool lc3::sim::run(void)
{
    simulator.simulate();
    // TODO: get return status
    return true;
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
    state.writeMem(addr + value.size(), 0);
    state.setMemLine(addr + value.size(), "");
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

lc3::utils::IPrinter & lc3::sim::getPrinter(void) { return printer; }
lc3::utils::IPrinter const & lc3::sim::getPrinter(void) const { return printer; }
lc3::utils::IInputter & lc3::sim::getInputter(void) { return inputter; }
lc3::utils::IInputter const & lc3::sim::getInputter(void) const { return inputter; }
void lc3::sim::setPrintLevel(uint32_t print_level) { simulator.setPrintLevel(print_level); }

void lc3::sim::loadOS(void)
{
    core::Assembler assembler(printer, 0, false);
    assembler.setFilename("lc3os");

    std::stringstream src_buffer;
    src_buffer << lc3::core::getOSSrc();
    std::shared_ptr<std::stringstream> obj_stream = assembler.assemble(src_buffer);
    simulator.loadObj("lc3os", *obj_stream);
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

    printer.print("attemping to assemble " + asm_filename + " into " + obj_filename);
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

    printer.print("attemping to convert " + bin_filename + " into " + obj_filename);
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
