#include "device_regs.h"
#include "device.h"
#include "state.h"

using namespace lc3::core;

MachineState::MachineState(void) : pc(0), ir(0), decoded_ir(nullptr), ssp(0)
{
    mem.resize(USER_END - SYSTEM_START + 1);
    rf.resize(16);
}

std::pair<uint16_t, PIMicroOp> MachineState::readMem(uint16_t addr) const
{
    // TODO: Check ACV
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

PIMicroOp MachineState::writeMem(uint16_t addr, uint16_t value)
{
    // TODO: Check ACV
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

std::string MachineState::getMemLine(uint16_t addr) const
{
    if(addr < MMIO_START) {
        return mem[addr].getLine();
    }

    return "";
}

void MachineState::setMemLine(uint16_t addr, std::string const & value)
{
    if(addr < MMIO_START) {
        mem[addr].setLine(value);
    }
}

void MachineState::registerDeviceReg(uint16_t mem_addr, PIDevice device)
{
    mmio[mem_addr] = device;
}
