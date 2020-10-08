#include "device.h"

using namespace lc3::core;

std::pair<uint16_t, PIMicroOp> RWReg::read(uint16_t addr)
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

KeyboardDevice::KeyboardDevice(lc3::utils::IInputter & inputter) : inputter(inputter)
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

std::pair<uint16_t, PIMicroOp> KeyboardDevice::read(uint16_t addr)
{
    if(addr == KBSR) {
        PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::INPUT_POLL);
        return std::make_pair(status.getValue(), callback);
    } else if(addr == KBDR) {
        uint16_t status_value = status.getValue();
        if(utils::getBit(status_value, 15) == 1) {
            PIMicroOp write_addr = std::make_shared<RegWriteImmMicroOp>(8, KBSR);
            PIMicroOp toggle_status = std::make_shared<MemWriteImmMicroOp>(8, status_value & 0x7FFF);
            PIMicroOp pop_from_buffer = std::make_shared<GenericPopMicroOp<std::queue<KeyInfo>>>(key_buffer, "kbbuf");
            write_addr->insert(toggle_status);
            toggle_status->insert(pop_from_buffer);
            return std::make_pair(data.getValue(), write_addr);
        } else {
            return std::make_pair(data.getValue(), nullptr);
        }
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
        key_buffer.emplace(c);
    }

    if(! key_buffer.empty()) {
        status.setValue(status.getValue() | 0x8000);
        data.setValue(static_cast<uint16_t>(key_buffer.front().value));

        if(! key_buffer.front().triggered_interrupt && (status.getValue() & 0x4000) == 0x4000) {
            key_buffer.front().triggered_interrupt = true;
            return std::make_shared<PushInterruptTypeMicroOp>(InterruptType::KEYBOARD);
        }
    }

    return nullptr;
}

std::pair<uint16_t, PIMicroOp> DisplayDevice::read(uint16_t addr)
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
