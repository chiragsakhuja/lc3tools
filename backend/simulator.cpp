#include "simulator.h"

#include <iostream>

#include "decoder.h"
#include "device_regs.h"

using namespace lc3::core;

Simulator::Simulator(void) : time(0)
{
    devices.emplace_back(std::make_shared<RWReg>(PSR));

    for(PIDevice dev : devices) {
        for(uint16_t dev_addr : dev->getAddrMap()) {
            state.registerDeviceReg(dev_addr, dev);
        }
    }

    // TODO: Create start event
    state.writePC(RESET_PC);
    // TODO: Start off in system mode, so initialize R6 to system stack pointer
    state.writeSSP(USER_START);
    state.writePSR(0x8002);

}

void Simulator::main(void)
{
    sim::Decoder decoder;
    state.writeReg(0, 0xdead);
    state.writeReg(1, 0xbeef);
    state.writeMemImm(0x25, 0x200);

    state.writeMemImm(0x200, 0x927f);
    state.writeMemImm(0x201, 0x3005);
    state.writeMemImm(0x202, 0xb204);
    state.writeMemImm(0x203, 0xa403);
    state.writeMemImm(0x204, 0xf025);
    events.emplace(std::make_shared<AtomicInstProcessEvent>(0, decoder));
    events.emplace(std::make_shared<AtomicInstProcessEvent>(10, decoder));
    events.emplace(std::make_shared<AtomicInstProcessEvent>(10, decoder));
    events.emplace(std::make_shared<AtomicInstProcessEvent>(10, decoder));
    events.emplace(std::make_shared<AtomicInstProcessEvent>(10, decoder));
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
