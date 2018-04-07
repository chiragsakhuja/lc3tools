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
        state.logger.print(std::string(1, char_value));
        state.console_buffer.push_back(char_value);
    } else if(addr == KBSR) {
        state.mem[addr].setValue(value & 0x4000);
        return;
    }

    state.mem[addr].setValue(value);
}

void lc3::core::SwapSPEvent::updateState(MachineState & state) const
{
    uint32_t old_sp = state.regs[6];
    state.regs[6] = state.backup_sp;
    state.backup_sp = old_sp;
}
