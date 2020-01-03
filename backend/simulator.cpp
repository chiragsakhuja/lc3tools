#include "simulator.h"

#include <iostream>

#include "decoder.h"
#include "device_regs.h"

using namespace lc3::core;

Simulator::Simulator(lc3::utils::IPrinter & printer, lc3::utils::IInputter & inputter, uint32_t print_level) :
    time(0), logger(printer, print_level), inputter(inputter)
{
    devices.emplace_back(std::make_shared<DisplayDevice>(logger));

    for(PIDevice dev : devices) {
        for(uint16_t dev_addr : dev->getAddrMap()) {
            state.registerDeviceReg(dev_addr, dev);
        }
    }
}

void Simulator::simulate(void)
{
    sim::Decoder decoder;
    events.emplace(std::make_shared<StartupEvent>());

    do {
        for(PIDevice dev : devices) {
            events.emplace(std::make_shared<DeviceUpdateEvent>(9, dev));
        }
        events.emplace(std::make_shared<AtomicInstProcessEvent>(10, decoder));

        mainLoop();
    } while(lc3::utils::getBit(state.readMCR(), 15) == 1);
}

void Simulator::loadObjFile(std::string filename, std::istream & buffer)
{
    events.emplace(std::make_shared<LoadObjFileEvent>(filename, buffer));

    mainLoop();
}

void Simulator::mainLoop(void)
{
    while(! events.empty()) {
        PIEvent event = events.top();
        events.pop();

        if(event != nullptr) {
            time += event->time_delta;
            logger.printf(lc3::utils::PrintType::P_EXTRA, true, "%d: %s", time, event->toString(state).c_str());
            event->handleEvent(state);

            PIMicroOp uop = event->uops;
            while(uop != nullptr) {
                logger.printf(lc3::utils::PrintType::P_EXTRA, true, "%d: |- %s", time, uop->toString(state).c_str());
                uop->handleMicroOp(state);
                uop = uop->getNext();
            }
        }

        logger.newline(lc3::utils::PrintType::P_EXTRA);
    }
}
