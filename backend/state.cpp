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

uint32_t lc3::core::MachineState::readMemEvent(uint32_t addr, bool & change_mem, std::shared_ptr<IEvent> & change) const
{
    assert(addr <= 0xFFFF);

    change_mem = false;
    change = nullptr;

    uint32_t value;
    if(addr == KBSR || addr == KBDR) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        value = readMemRaw(addr);
        if(addr == KBDR) {
            change_mem = true;
            change = std::make_shared<MemWriteEvent>(KBSR, readMemRaw(KBSR) & 0x7FFF);
        } else if(addr == KBSR && (value & 0x8000) == 0) {
            change_mem = true;
            change = std::make_shared<CallbackEvent>(input_poll_callback_v, input_poll_callback);
        }
    } else {
        value = readMemRaw(addr);
    }
    return value;
}

uint32_t lc3::core::MachineState::readMemSafe(uint32_t addr)
{
    bool change_mem;
    std::shared_ptr<IEvent> change;

    uint32_t value = readMemEvent(addr, change_mem, change);

    if(change_mem) {
        change->updateState(*this);
    }

    return value;
}

uint32_t lc3::core::MachineState::readMemRaw(uint32_t addr) const {
    assert(addr <= 0xFFFF);

    return mem[addr].getValue();
}

void lc3::core::MachineState::writeMemEvent(uint32_t addr, uint16_t value, bool & change_mem,
    std::shared_ptr<IEvent> & change)
{
    assert(addr <= 0xFFFF);

    change_mem = false;
    change = nullptr;

    if(addr == PSR) {
        uint32_t current_psr = readMemRaw(PSR);
        if(((current_psr & 0x8000) ^ (value & 0x8000)) == 0x8000) {
            change_mem = true;
            change = std::make_shared<SwapSPEvent>();
        }
    }

    writeMemRaw(addr, value);
}

void lc3::core::MachineState::writeMemSafe(uint32_t addr, uint16_t value)
{
    bool change_mem;
    std::shared_ptr<IEvent> change;

    writeMemEvent(addr, value, change_mem, change);

    if(change_mem) {
        change->updateState(*this);
    }
}

void lc3::core::MachineState::writeMemRaw(uint32_t addr, uint16_t value)
{
    assert(addr <= 0xFFFF);

    mem[addr].setValue(value);
}

void lc3::core::MemWriteEvent::updateState(MachineState & state) const
{
    assert(addr < 0xFFFF);

    if(addr == DDR) {
        char char_value = (char) (value & 0xFF);
        if(char_value == 10 || char_value == 13) {
            state.logger.newline();
        } else {
            state.logger.print(std::string(1, char_value));
        }
    } else if(addr == KBSR) {
        std::lock_guard<std::mutex> guard(g_io_lock);
        state.writeMemSafe(addr, value & 0x4000);
        return;
    }

    state.writeMemSafe(addr, value);
}

void lc3::core::SwapSPEvent::updateState(MachineState & state) const
{
    uint32_t old_sp = state.regs[6];
    state.regs[6] = state.readMemRaw(BSP);
    state.writeMemRaw(BSP, old_sp);
}
