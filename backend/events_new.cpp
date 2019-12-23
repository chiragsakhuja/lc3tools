#include "events_new.h"
#include "state_new.h"

#include "utils.h"

namespace lc3
{
namespace core
{
    void MemReadEvent::handleEvent(MachineState & state) const
    {
        state.writeReg(reg_id, state.getMemValue(mem_addr));
    }

    std::string MemReadEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("R%d:0x%0.4x <= MEM[0x%0.4x]:0x%0.4x", reg_id, state.readReg(reg_id), mem_addr,
            state.getMemValue(mem_addr));
    }

    void MemWriteImmEvent::handleEvent(MachineState & state) const
    {
        state.setMemValue(addr, value);
    }

    std::string MemWriteImmEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("MEM[0x%0.4x]:0x%0.4x <= 0x%0.4x", addr, state.getMemValue(addr), value);
    }
};
};
