#ifndef STATE_H
#define STATE_H

namespace core
{
    struct MachineState
    {
        std::vector<uint16_t> mem;
        std::array<uint32_t, 8> regs;
        uint32_t pc;
        uint32_t psr;
    };
};

#endif
