#include <array>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "tokens.h"

#include "printer.h"
#include "logger.h"

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

    state.mem.resize(1 << 16);

    state.pc = RESET_PC;
    state.psr = 0x0002;

    state.mem[MCR] = 0x8000;  // indicate the machine is running
}

void Simulator::simulate(void)
{
    uint32_t encoded_inst = state.mem[state.pc];

    Instruction * candidate;
    bool valid = decoder.findInstructionByEncoding(encoded_inst, candidate);
    if(valid) {
        decoder.decode(encoded_inst, *candidate);
        state.pc += 1;
        candidate->execute(state);
        delete candidate;
    } else {
        throw std::runtime_error("invalid instruction");
    }
}

void Simulator::loadObjectFile(std::string const & filename)
{
    std::ifstream file(filename, std::ios::binary);
    if(!file) {
        logger.printf(PRINT_TYPE_WARNING, true, "skipping file %s ...", filename.c_str());
    } else {
        std::istreambuf_iterator<char> it(file);
        uint32_t orig = *it;
        ++it;
        orig = (orig << 8) | (*it);
        ++it;
        uint32_t cur = orig;
        while(it != std::istreambuf_iterator<char>()) {
            uint32_t value = (uint8_t) *it;
            ++it;
            value = (value << 8) | ((uint8_t) (*it));
            ++it;
            state.mem[cur] = value;
            ++cur;
        }
        state.pc = orig;
        simulate();
    }
}
