#include "device_regs.h"
#include "device_new.h"
#include "state_new.h"

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

    PIEvent MachineState::readMem(uint16_t reg_id, uint16_t mem_addr)
    {
        if(MMIO_START <= mem_addr && mem_addr <= MMIO_END) {
            auto search = mmio.find(mem_addr);
            if(search != mmio.end()) {
                return search->second->read(reg_id, mem_addr);
            } else {
                return nullptr;
            }
        } else {
            return std::make_shared<MemReadEvent>(reg_id, mem_addr);
        }
    }

    uint16_t MachineState::getMemValue(uint16_t addr) const
    {
        if(MMIO_START <= addr && addr <= MMIO_END) {
            auto search = mmio.find(addr);
            if(search != mmio.end()) {
                return search->second->getValue(addr);
            } else {
                return 0x0000;
            }
        } else {
            return mem[addr].getValue();
        }
    }

    void MachineState::setMemValue(uint16_t addr, uint16_t value)
    {
        if(MMIO_START <= addr && addr <= MMIO_END) {
            auto search = mmio.find(addr);
            if(search != mmio.end()) {
                search->second->setValue(addr, value);
            }
        } else {
            return mem[addr].setValue(value);
        }
    }
};
};
