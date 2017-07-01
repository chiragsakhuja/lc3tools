#include <fstream>
#include <cstdint>
#include <array>
#include <vector>

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "simulator.h"

using namespace core;

Simulator::Simulator(bool log_enable, utils::Printer & printer)
{
    this->log_enable = log_enable;
    this->logger = new Logger(printer);

    state.mem.resize(1 << 16);
    state.pc = 0;
    state.psr = 0x0002;
}

Simulator::~Simulator(void)
{
    delete logger;
};

void Simulator::simulate(void)
{
    
}

void Simulator::loadObjectFile(std::string const & filename)
{
    std::ifstream file(filename, std::ios::binary);
    if(!file) {
        logger->printf(PRINT_TYPE_WARNING, true, "skipping file %s ...", filename.c_str());
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
