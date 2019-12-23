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
        events.emplace(state.readMem(1, KBDR));
        while(time < 100 && ! events.empty()) {
            PIEvent event = events.top();
            events.pop();

            if(event != nullptr) {
                time += event->time_delta;
            }

            while(event != nullptr) {
                std::cout << time << ": " << event->toString(state) << '\n';
                event->handleEvent(state);
                event = event->next;
            }

            if(events.empty()) {
                if(time % 20 == 0) {
                    events.emplace(std::make_shared<MemReadEvent>(10, 1, 0x1000));
                } else {
                    events.emplace(std::make_shared<MemWriteImmEvent>(10, 0x1000, time & 0xFFFF));
                }
            }
        }
    }
};
};
