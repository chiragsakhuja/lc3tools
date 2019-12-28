#include "simulator.h"

#include <iostream>

#include "decoder.h"
#include "device_regs.h"

using namespace lc3::core;

SimulatorNew::SimulatorNew(void) : time(0)
{
    state.writePC(RESET_PC);
}

void SimulatorNew::main(void)
{
    sim::Decoder decoder;
    state.writeMemImm(0x200, 0x1000);
    events.emplace(std::make_shared<AtomicInstProcessEvent>(decoder));
    while(! events.empty()) {
        PIEvent event = events.top();
        events.pop();

        if(event != nullptr) {
            time += event->time_delta;
            std::cout << time << ": " << event->toString(state) << '\n';
            event->handleEvent(state);

            PIMicroOp uop = event->uops;
            while(uop != nullptr) {
                std::cout << time << ": |- " << uop->toString(state) << '\n';
                uop->handleMicroOp(state);
                uop = uop->getNext();
            }
        }
    }
}
