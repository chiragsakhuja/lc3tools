#include "device.h"

using namespace lc3::core;

std::pair<uint16_t, PIMicroOp> RWReg::read(uint16_t addr) const
{
    if(addr == data_addr) {
        return std::make_pair(data.getValue(), nullptr);
    }

    return std::make_pair(0x0000, nullptr);
}

PIMicroOp RWReg::write(uint16_t addr, uint16_t value)
{
    if(addr == data_addr) {
        data.setValue(value);
    }

    return nullptr;
}

std::vector<uint16_t> RWReg::getAddrMap(void) const
{
    return { data_addr };
}

std::pair<uint16_t, PIMicroOp> KeyboardDevice::read(uint16_t addr) const
{
    if(addr == KBSR) {
        return std::make_pair(status.getValue(), nullptr);
    } else if(addr == KBDR) {
        uint16_t status_value = status.getValue();
        PIMicroOp toggle_status = std::make_shared<MemWriteImmMicroOp>(KBSR, status_value & 0x7FFF);
        return std::make_pair(data.getValue(), toggle_status);
    }

    return std::make_pair(0x0000, nullptr);
}

PIMicroOp KeyboardDevice::write(uint16_t addr, uint16_t value)
{
    if(addr == KBSR) {
        status.setValue(value & 0x4000);
    }

    return nullptr;
}

std::vector<uint16_t> KeyboardDevice::getAddrMap(void) const
{
    return { KBSR, KBDR };
}
