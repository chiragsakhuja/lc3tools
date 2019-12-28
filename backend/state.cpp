#include "device_regs.h"
#include "device.h"
#include "state.h"

namespace lc3
{
namespace core
{
    MachineState::MachineState(void)
    {
        mem.resize(USER_END - SYSTEM_START + 1);
        rf.resize(8);
        PIDevice keyboard = std::make_shared<KeyboardDevice>();
        mmio[KBSR] = keyboard;
        mmio[KBDR] = keyboard;
    }

    std::pair<uint16_t, PIMicroOp> MachineState::readMem(uint16_t addr) const
    {
        if(MMIO_START <= addr && addr <= MMIO_END) {
            auto search = mmio.find(addr);
            if(search != mmio.end()) {
                return search->second->read(addr);
            } else {
                return std::make_pair(0x0000, nullptr);
            }
        } else {
            return std::make_pair(mem[addr].getValue(), nullptr);
        }
    }

    PIMicroOp MachineState::writeMemImm(uint16_t addr, uint16_t value)
    {
        if(MMIO_START <= addr && addr <= MMIO_END) {
            auto search = mmio.find(addr);
            if(search != mmio.end()) {
                return search->second->write(addr, value);
            }
        } else {
            mem[addr].setValue(value);
        }

        return nullptr;
    }
};
};
