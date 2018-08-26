#include <cassert>
#include <mutex>

#include "device_regs.h"
#include "state.h"

namespace lc3
{
namespace core
{
    extern std::mutex g_io_lock;
};
};

uint32_t lc3::core::MachineState::readMem(uint32_t addr, bool & change_mem, std::shared_ptr<IEvent> & change) const
{
    assert(addr < 0xFFFF);

    change_mem = false;
    change = nullptr;

    uint32_t value;
    if(addr == KBSR || addr == KBDR) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        value = mem[addr].getValue();
        if(addr == KBDR) {
            change_mem = true;
            change = std::make_shared<MemWriteEvent>(KBSR, mem[KBSR].getValue() & 0x7FFF);
        }
    } else {
        value = mem[addr].getValue();
    }
    return value;
}

void lc3::core::MemWriteEvent::updateState(MachineState & state) const
{
    assert(addr < 0xFFFF);

    if(addr == DDR) {
        char char_value = (char) (value & 0xFF);
        if(char_value == 0xa) {
            state.logger.newline();
        } else {
            state.logger.print(std::string(1, char_value));
        }
    } else if(addr == KBSR) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        state.mem[addr].setValue(value & 0x4000);
        return;
    }

    state.mem[addr].setValue(value);
}

void lc3::core::SwapSPEvent::updateState(MachineState & state) const
{
    if(shouldSwap(state)) {
        uint32_t old_sp = state.regs[6];
        state.regs[6] = state.backup_sp;
        state.backup_sp = old_sp;
    }
}

bool lc3::core::SwapSPEvent::shouldSwap(MachineState const & state) const
{
    bool target_ssp = target == MachineState::SPType::SSP;
    bool system_mode = (state.mem[PSR].getValue() & 0x8000) == 0x0000;
    return target_ssp ^ system_mode;
}
