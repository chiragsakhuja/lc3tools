#include "simulator.h"

#include <iostream>

#include "device_regs.h"

namespace lc3
{
namespace core
{
    SimulatorNew::SimulatorNew(void) : time(0)
    {
        state.writePC(RESET_PC);
    }

    void SimulatorNew::main(void)
    {
        events.emplace(std::make_shared<AtomicInstProcessEvent>());
        while(! events.empty()) {
            PIEvent event = events.top();
            events.pop();

            if(event != nullptr) {
                time += event->time_delta;
            }

            std::string prefix = "";
            while(event != nullptr) {
                std::cout << time << ": " << prefix << event->toString(state) << '\n';
                event->handleEvent(state);
                event = event->next;
                prefix = "  ";
            }

            /*
             *if(events.empty()) {
             *    if(time % 20 == 0) {
             *        events.emplace(std::make_shared<MemReadEvent>(10, 1, 0x1000));
             *    } else {
             *        events.emplace(std::make_shared<MemWriteImmEvent>(10, 0x1000, time & 0xFFFF));
             *    }
             *}
             */
        }
    }
};
};
