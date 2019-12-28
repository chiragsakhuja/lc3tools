#include "event.h"
#include "uop.h"
#include "state.h"

#include "utils.h"

using namespace lc3::core;

void AtomicInstProcessEvent::handleEvent(MachineState & state)
{
    PIMicroOp fetch = std::make_shared<FetchMicroOp>();
    PIMicroOp inc_pc = std::make_shared<PCAddImmMicroOp>(1);
    PIMicroOp decode = std::make_shared<DecodeMicroOp>(decoder);

    fetch->insert(inc_pc);
    inc_pc->insert(decode);
    uops = fetch;
}

std::string AtomicInstProcessEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("Processing M[0x%0.4x]:0x%0.4x", state.readPC(), std::get<0>(state.readMem(state.readPC())));
}
