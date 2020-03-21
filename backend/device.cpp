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

KeyboardDevice::KeyboardDevice(lc3::utils::IInputter & inputter) : inputter(inputter), interrupt_triggered(false)
{
    status.setValue(0x0000);
    data.setValue(0x0000);
}

void KeyboardDevice::startup(void)
{
    inputter.beginInput();
}

void KeyboardDevice::shutdown(void)
{
    inputter.endInput();
}

std::pair<uint16_t, PIMicroOp> KeyboardDevice::read(uint16_t addr) const
{
    if(addr == KBSR) {
        return std::make_pair(status.getValue(), nullptr);
    } else if(addr == KBDR) {
        uint16_t status_value = status.getValue();
        PIMicroOp write_addr = std::make_shared<RegWriteImmMicroOp>(8, KBSR);
        PIMicroOp toggle_status = std::make_shared<MemWriteImmMicroOp>(8, status_value & 0x7FFF);
        write_addr->insert(toggle_status);

        return std::make_pair(data.getValue(), write_addr);
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

PIMicroOp KeyboardDevice::tick(void)
{
    // Set ready bit.
    char c;
    if(inputter.getChar(c)) {
        status.setValue(status.getValue() | 0x8000);
        data.setValue(static_cast<uint16_t>(c));
        interrupt_triggered = false;
    }

    // Trigger interrupt.
    if(! interrupt_triggered && (status.getValue() & 0xC000) == 0xC000) {
        interrupt_triggered = true;
        return std::make_shared<PushInterruptTypeMicroOp>(InterruptType::KEYBOARD);
    }

    return nullptr;
}

std::pair<uint16_t, PIMicroOp> DisplayDevice::read(uint16_t addr) const
{
    if(addr == DSR) {
        return std::make_pair(status.getValue(), nullptr);
    }

    return std::make_pair(0x0000, nullptr);
}

DisplayDevice::DisplayDevice(lc3::utils::Logger & logger) : logger(logger)
{
    status.setValue(0x0000);
    data.setValue(0x0000);
}

PIMicroOp DisplayDevice::write(uint16_t addr, uint16_t value)
{
    if(addr == DSR) {
        status.setValue(value & 0x4000);
    } else if(addr == DDR) {
        // Clear ready bit.
        status.setValue(status.getValue() & 0x7FFF);

        // Write to DDR and output to screen.
        data.setValue(value & 0x00FF);
        char char_value = static_cast<char>(value & 0x00FF);
        if(char_value == 10 || char_value == 13) {
            logger.newline(utils::PrintType::P_NONE);
        } else {
            logger.print(std::string(1, char_value));
        }
    }

    return nullptr;
}

std::vector<uint16_t> DisplayDevice::getAddrMap(void) const
{
    return { DSR, DDR };
}

PIMicroOp DisplayDevice::tick(void)
{
    // Set ready bit.
    status.setValue(status.getValue() | 0x8000);

    return nullptr;
}
