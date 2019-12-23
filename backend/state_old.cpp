/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cassert>
#include <mutex>

#include "device_regs.h"
#include "state_old.h"

namespace lc3
{
namespace core_old
{
    extern std::mutex g_io_lock;
};
};

uint32_t lc3::core_old::MachineState::readMemEvent(uint32_t addr, bool & change_mem, std::shared_ptr<IEvent> & change) const
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xFFFF);
#endif

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
            change = std::make_shared<CallbackEvent>(wait_for_input_callback_v, wait_for_input_callback);
        }
    } else {
        value = readMemRaw(addr);
    }
    return value;
}

uint32_t lc3::core_old::MachineState::readMemSafe(uint32_t addr)
{
    bool change_mem;
    std::shared_ptr<IEvent> change;

    uint32_t value = readMemEvent(addr, change_mem, change);

    if(change_mem) {
        change->updateState(*this);
    }

    return value;
}

uint32_t lc3::core_old::MachineState::readMemRaw(uint32_t addr) const
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xFFFF);
#endif

    return mem[addr].getValue();
}

void lc3::core_old::MachineState::writeMemEvent(uint32_t addr, uint16_t value, bool & change_mem,
    std::shared_ptr<IEvent> & change)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xFFFF);
#endif

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

void lc3::core_old::MachineState::writeMemSafe(uint32_t addr, uint16_t value)
{
    bool change_mem;
    std::shared_ptr<IEvent> change;

    writeMemEvent(addr, value, change_mem, change);

    if(change_mem) {
        change->updateState(*this);
    }
}

void lc3::core_old::MachineState::writeMemRaw(uint32_t addr, uint16_t value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xFFFF);
#endif

    mem[addr].setValue(value);
}

void lc3::core_old::MemWriteEvent::updateState(MachineState & state) const
{
#ifdef _ENABLE_DEBUG
    assert(addr < 0xFFFF);
#endif

    if(addr == DDR) {
        char char_value = (char) (value & 0xFF);
        if(char_value == 10 || char_value == 13) {
            state.logger.newline(utils::PrintType::P_NONE);
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

void lc3::core_old::SwapSPEvent::updateState(MachineState & state) const
{
    uint32_t old_sp = state.regs[6];
    state.regs[6] = state.readMemRaw(BSP);
    state.writeMemRaw(BSP, old_sp);
}
