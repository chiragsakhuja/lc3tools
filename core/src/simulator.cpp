#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "configured_paths.h"

#include "utils.h"

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "object_file_utils.h"

#include "state.h"

#include "instructions.h"
#include "instruction_decoder.h"

#include "device_regs.h"

#include "simulator.h"

using namespace core;

Simulator::Simulator(bool log_enable, utils::IPrinter & printer) :
    state(logger), logger(printer)
{
    this->log_enable = log_enable;
    this->state.mem.resize(1 << 16);
    this->reset();
    this->loadObjectFile(std::string(GLOBAL_RES_PATH) + "/lc3os.obj");
    this->state.pc = RESET_PC;
}

void Simulator::reset(void)
{
    for(uint32_t i = 0; i < 8; i += 1) {
        state.regs[i] = 0;
    }

    state.pc = RESET_PC;
    state.psr = 0x8002;

    for(uint32_t i = 0; i < (1 << 16); i += 1) {
        state.mem[i] = 0;
    }
    state.mem[MCR] = 0x8000;  // indicate the machine is running
}

void Simulator::updateDevices(void)
{
    state.mem[DSR] |= 0x8000;
}

void Simulator::executeInstruction(void)
{
    uint32_t encoded_inst = state.mem[state.pc];

    IInstruction * candidate;
    bool valid = decoder.findInstructionByEncoding(encoded_inst, candidate);
    if(valid) {
        candidate->assignOperands(encoded_inst);
        if(log_enable) {
            logger.printf(PRINT_TYPE_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc, candidate->toValueString().c_str(), encoded_inst);
        }
        state.pc += 1;
        std::vector<IStateChange const *> changes = candidate->execute(state);
        delete candidate;

        for(uint32_t i = 0; i < changes.size(); i += 1) {
            if(log_enable) {
                logger.printf(PRINT_TYPE_EXTRA, false, "  %s", changes[i]->getOutputString(state).c_str());
            }
            changes[i]->updateState(state);

            delete changes[i];
        }
    } else {
        throw std::runtime_error(core::ssprintf("invalid instruction 0x%0.4x", encoded_inst));
    }
}

void Simulator::simulate(void)
{
    while((state.mem[MCR] & 0x8000) != 0) {
        executeInstruction();
        updateDevices();
    }
    /*
     *for(char i : state.console_buffer) {
     *    if(i == '\n') {
     *        logger.newline();
     *    } else {
     *        logger.print(std::string(1, i));
     *    }
     *}
     */
}

void Simulator::loadObjectFile(std::string const & filename)
{
    utils::ObjectFileReader reader(filename);
    uint32_t offset = 0;
    while(! reader.atEnd()) {
        utils::ObjectFileStatement statement = reader.readStatement();
        if(statement.isOrig()) {
            state.pc = statement.getValue();
            offset = 0;
        } else {
            state.mem[state.pc + offset] = statement.getValue();
            offset += 1;
        }
    }
    state.mem[MCR] |= 0x8000;
}
