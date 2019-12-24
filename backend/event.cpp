#include "event.h"
#include "state.h"

#include "utils.h"

namespace lc3
{
namespace core
{
    void AtomicInstProcessEvent::handleEvent(MachineState & state)
    {
        next = std::make_shared<FetchEvent>();
        PIEvent cur = next;
        cur->next = std::make_shared<PCAddImmEvent>(1);
    }

    std::string AtomicInstProcessEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("Processing M[0x%0.4x]:0x%0.4x", state.readPC(), state.getMemValue(state.readPC()));
    }

    void FetchEvent::handleEvent(MachineState & state)
    {
        state.writeIR(state.getMemValue(state.readPC()));
    }

    std::string FetchEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("IR:0x%0.4x <= M[0x%0.4x]:0x%0.4x", state.readIR(), state.readPC(),
            state.getMemValue(state.readPC()));
    }

    void PCAddImmEvent::handleEvent(MachineState & state)
    {
        state.writePC(state.readPC() + amnt);
    }

    std::string PCAddImmEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("PC:0x%0.4x <= (PC:0x%0.4x + #%d):0x%0.4x", state.readPC(), state.readPC(), amnt,
            state.readPC() + amnt);
    }

    void MemReadEvent::handleEvent(MachineState & state)
    {
        state.writeReg(reg_id, state.getMemValue(mem_addr));
    }

    std::string MemReadEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("R%d:0x%0.4x <= MEM[0x%0.4x]:0x%0.4x", reg_id, state.readReg(reg_id), mem_addr,
            state.getMemValue(mem_addr));
    }

    void MemWriteImmEvent::handleEvent(MachineState & state)
    {
        state.setMemValue(addr, value);
    }

    std::string MemWriteImmEvent::toString(MachineState const & state) const
    {
        return utils::ssprintf("MEM[0x%0.4x]:0x%0.4x <= 0x%0.4x", addr, state.getMemValue(addr), value);
    }
};
};
