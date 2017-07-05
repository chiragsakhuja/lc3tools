#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "device_regs.h"

#include "state.h"

using namespace core;

void MemStateChange::updateState(MachineState & state) const
{
    state.mem[addr] = value; 
    if(addr == DDR) {
        char char_value = (char) (value & 0xff);
        state.logger.print(std::string(1, char_value));
        state.console_buffer.push_back(char_value);
    }
}
