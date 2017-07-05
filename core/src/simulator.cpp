#include <array>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "utils.h"

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "object_file_utils.cpp"

#include "state.h"

#include "instructions.h"
#include "instruction_decoder.h"

#include "simulator.h"

using namespace core;

#define RESET_PC 0x0200
#define MCR 0xFFFE

Simulator::Simulator(bool log_enable, utils::IPrinter & printer) :
    logger(printer)
{
    this->log_enable = log_enable;

    for(uint32_t i = 0; i < 8; i += 1) {
        state.regs[i] = 0;
    }

    state.pc = RESET_PC;
    state.psr = 0x8002;

    state.mem.resize(1 << 16);
    for(uint32_t i = 0; i < (1 << 16); i += 1) {
        state.mem[i] = 0;
    }
    state.mem[MCR] = 0x8000;  // indicate the machine is running
}

void Simulator::simulate(void)
{
    for(int i = 0; i < 200; i += 1) {
        uint32_t encoded_inst = state.mem[state.pc];

        Instruction * candidate;
        bool valid = decoder.findInstructionByEncoding(encoded_inst, candidate);
        if(valid) {
            decoder.decode(encoded_inst, *candidate);
            state.pc += 1;
            std::vector<IStateChange const *> changes = candidate->execute(state);
            if(log_enable) {
                logger.printf(PRINT_TYPE_EXTRA, true, "executing PC 0x%0.4x: %s (0x%0.4x)", state.pc, candidate->toValueString().c_str(), encoded_inst);
            }
            delete candidate;

            for(uint32_t i = 0; i < changes.size(); i += 1) {
                if(log_enable) {
                    logger.printf(PRINT_TYPE_EXTRA, false, "  %s", changes[i]->getOutputString(state).c_str());
                }
                changes[i]->updateState(state);

                delete changes[i];
            }
        } else {
            throw std::runtime_error("invalid instruction");
        }
    }
}

void Simulator::loadObjectFile(std::string const & filename)
{
    ObjectFileReader reader(filename);
    uint32_t offset = 0;
    while(! reader.atEnd()) {
        ObjectFileStatement statement = reader.readStatement();
        if(statement.isOrig()) {
            state.pc = statement.getValue();
            offset = 0;
        } else {
            state.mem[state.pc + offset] = statement.getValue();
            offset += 1;
        }
    }
}
