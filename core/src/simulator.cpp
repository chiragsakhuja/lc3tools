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

#include "statement.h"

#include "state.h"

#include "instructions.h"
#include "instruction_decoder.h"

#include "device_regs.h"

#include "simulator.h"

using namespace core;

void Simulator::loadObjectFile(std::string const & filename)
{
    std::ifstream file(filename);
    if(! file) {
        logger.printf(PRINT_TYPE_ERROR, true, "could not open file \'%s\' for reading", filename.c_str());
        throw core::exception("could not open file");
    }

    uint32_t offset = 0;
    while(! file.eof()) {
        utils::Statement statement;
        file >> statement;

        if(statement.isOrig()) {
            state.pc = statement.getValue();
            offset = 0;
        } else {
            logger.printf(PRINT_TYPE_EXTRA, true, "0x%0.4x: %s (0x%0.4x)", state.pc + offset,
                statement.getLine().c_str(), statement.getValue());
            state.mem[state.pc + offset] = statement;
            offset += 1;
        }

    }
    uint16_t value = state.mem[MCR].getValue();
    state.mem[MCR].setValue(value | 0x8000);
}

void Simulator::loadOS(void)
{
    loadObjectFile(std::string(GLOBAL_RES_PATH) + "/lc3os.obj");
    state.pc = RESET_PC;
}

Simulator::Simulator(bool log_enable, utils::IPrinter & printer) : state(logger), logger(log_enable, printer)
{
    state.mem.resize(1 << 16);
    reset();
}

void Simulator::reset(void)
{
    for(uint32_t i = 0; i < 8; i += 1) {
        state.regs[i] = 0;
    }

    state.pc = RESET_PC;
    state.psr = 0x8002;

    for(uint32_t i = 0; i < (1 << 16); i += 1) {
        state.mem[i].setValue(0);
    }

    state.mem[MCR].setValue(0x8000);  // indicate the machine is running
    state.running = true;
}

void Simulator::executeInstruction(void)
{
    uint32_t encoded_inst = state.mem[state.pc].getValue();

    IInstruction * candidate;
    if(! decoder.findInstructionByEncoding(encoded_inst, candidate)) {
        logger.printf(PRINT_TYPE_ERROR, true, "invalid instruction 0x%0.4x", encoded_inst);
        throw core::exception("invalid instruction");
    }

    candidate->assignOperands(encoded_inst);
    logger.printf(PRINT_TYPE_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc,
        state.mem[state.pc].getLine().c_str(), encoded_inst);
    state.pc += 1;
    std::vector<IStateChange const *> changes = candidate->execute(state);
    delete candidate;

    for(uint32_t i = 0; i < changes.size(); i += 1) {
        logger.printf(PRINT_TYPE_EXTRA, false, "  %s", changes[i]->getOutputString(state).c_str());
        changes[i]->updateState(state);

        delete changes[i];
    }
}

void Simulator::simulate(void)
{
    while(state.running && (state.mem[MCR].getValue() & 0x8000) != 0) {
        executeInstruction();
        updateDevices();
    }
    state.running = false;
}

void Simulator::updateDevices(void)
{
    uint16_t value = state.mem[DSR].getValue();
    state.mem[DSR].setValue(value | 0x8000);
}
