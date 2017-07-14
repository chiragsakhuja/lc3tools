#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "statement.h"

#include "device_regs.h"

#include "state.h"

namespace core
{
    extern std::mutex g_io_lock;
};

uint32_t core::MachineState::readMem(uint32_t addr, bool & change_mem, IStateChange *& change) const
{
    change_mem = false;
    change = nullptr;

    uint32_t value;
    if(addr == KBSR || addr == KBDR) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        value = mem[addr].getValue();
        if(addr == KBDR) {
            change_mem = true;
            change = new MemWriteStateChange(KBSR, mem[KBSR].getValue() & 0x7FFF);
        }
    } else {
        value = mem[addr].getValue();
    }
    return value;
}

void core::MemWriteStateChange::updateState(MachineState & state) const
{
    if(addr == DDR) {
        char char_value = (char) (value & 0xff);
        state.logger.print(std::string(1, char_value));
        state.console_buffer.push_back(char_value);
    }

    state.mem[addr].setValue(value);
}

void core::SwapSPStateChange::updateState(MachineState & state) const
{
    uint32_t old_sp = state.regs[7];
    state.regs[7] = state.backup_sp;
    state.backup_sp = old_sp;
}
