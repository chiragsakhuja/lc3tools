#include "event.h"
#include "uop.h"
#include "state.h"

#include "utils.h"

using namespace lc3::core;

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
