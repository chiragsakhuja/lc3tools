#include "device.h"

namespace lc3
{
namespace core
{
    PIEvent KeyboardDevice::read(uint16_t reg_id, uint16_t mem_addr)
    {
        if(mem_addr == KBSR) {
            return std::make_shared<MemReadEvent>(reg_id, mem_addr);
        } else if(mem_addr == KBDR) {
            uint16_t status_value = status.getValue();
            PIEvent ret = std::make_shared<MemReadEvent>(reg_id, mem_addr);
            ret->next = std::make_shared<MemWriteImmEvent>(KBSR, status_value & 0x7FFF);
            return ret;
        } else {
            return nullptr;
        }
    }

    uint16_t KeyboardDevice::getValue(uint16_t addr) const
    {
        if(addr == KBSR) {
            return status.getValue();
        } else if(addr == KBDR) {
            return data.getValue();
        } else {
            // This should never happen if the device is registered properly, but return 0 just in case.
            return 0x0000;
        }
    }

    void KeyboardDevice::setValue(uint16_t addr, uint16_t value)
    {
        if(addr == KBSR) {
            status.setValue(value);
        } else if(addr == KBDR) {
            return data.setValue(value);
        }
    }
};
};

