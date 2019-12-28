#include "device.h"

namespace lc3
{
namespace core
{
    std::pair<uint16_t, PIMicroOp> KeyboardDevice::read(uint16_t addr) const
    {
        if(addr == KBSR) {
            return std::make_pair(status.getValue(), nullptr);
        } else if(addr == KBDR) {
            uint16_t status_value = status.getValue();
            PIMicroOp toggle_status = std::make_shared<MemWriteImmMicroOp>(KBSR, status_value & 0x7FFF);
            return std::make_pair(data.getValue(), toggle_status);
        } else {
            return std::make_pair(0x0000, nullptr);
        }
    }

    PIMicroOp KeyboardDevice::write(uint16_t addr, uint16_t value)
    {
        if(addr == KBSR) {
            status.setValue(value);
        } else if(addr == KBDR) {
            data.setValue(value);
        }

        return nullptr;
    }
};
};

