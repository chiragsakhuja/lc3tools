#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
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

uint32_t core::MachineState::readMem(uint32_t addr, bool & change_mem, IEvent *& change) const
{
    assert(addr < 0xffff);

    change_mem = false;
    change = nullptr;

    uint32_t value;
    if(addr == KBSR || addr == KBDR) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        value = mem[addr].getValue();
        if(addr == KBDR) {
            change_mem = true;
            change = new MemWriteEvent(KBSR, mem[KBSR].getValue() & 0x7FFF);
        }
    } else {
        value = mem[addr].getValue();
    }
    return value;
}

void core::MemWriteEvent::updateState(MachineState & state) const
{
    assert(addr < 0xffff);

    if(addr == DDR) {
        char char_value = (char) (value & 0xff);
        state.logger.print(std::string(1, char_value));
        state.console_buffer.push_back(char_value);
    } else if(addr == KBSR) {
        state.mem[addr].setValue(value & 0x4000);
        return;
    }

    state.mem[addr].setValue(value);
}

void core::SwapSPEvent::updateState(MachineState & state) const
{
    uint32_t old_sp = state.regs[6];
    state.regs[6] = state.backup_sp;
    state.backup_sp = old_sp;
}
