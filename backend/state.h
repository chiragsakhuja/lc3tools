#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include "aliases.h"
#include "device.h"
#include "mem_new.h"

namespace lc3
{
namespace core
{
    class IEvent;
    using PIEvent = std::shared_ptr<IEvent>;

    class MachineState
    {
    private:
        std::vector<MemLocation> mem;
        std::vector<uint16_t> rf;
        std::unordered_map<uint16_t, PIDevice> mmio;
        uint16_t pc, ir;

    public:
        MachineState(void);

        uint16_t readPC(void) const { return pc; }
        void writePC(uint16_t value) { pc = value; }
        uint16_t readIR(void) const { return ir; }
        void writeIR(uint16_t value) { ir = value; }
        uint16_t readReg(uint16_t id) const { return rf[id]; }
        void writeReg(uint16_t id, uint16_t value) { rf[id] = value; }
        std::pair<uint16_t, PIMicroOp> readMem(uint16_t addr) const;
        PIMicroOp writeMemImm(uint16_t addr, uint16_t value);
    };
};
};

#endif
