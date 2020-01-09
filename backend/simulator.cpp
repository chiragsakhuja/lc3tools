#include "simulator.h"

#include <iostream>

#include "decoder.h"
#include "device_regs.h"

using namespace lc3::core;

Simulator::Simulator(lc3::utils::IPrinter & printer, lc3::utils::IInputter & inputter, uint32_t print_level) :
    time(0), logger(printer, print_level), inputter(inputter)
{
    devices.emplace_back(std::make_shared<KeyboardDevice>(inputter));
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
    events.emplace(std::make_shared<StartupEvent>(time));

    for(PIDevice dev : devices) {
        dev->startup();
    }

    do {
        for(PIDevice dev : devices) {
            events.emplace(std::make_shared<DeviceUpdateEvent>(time + 9, dev));
        }
        events.emplace(std::make_shared<AtomicInstProcessEvent>(time + 10, decoder));

        mainLoop();
    } while(lc3::utils::getBit(state.readMCR(), 15) == 1);

    for(PIDevice dev : devices) {
        dev->shutdown();
    }
}

void Simulator::loadObjFile(std::string filename, std::istream & buffer)
{
    events.emplace(std::make_shared<LoadObjFileEvent>(time + 1, filename, buffer));

    mainLoop();
}

void Simulator::mainLoop(void)
{
    while(! events.empty()) {
        PIEvent event = events.top();
        events.pop();

        if(event != nullptr) {
            if(event->time < time) {
                logger.printf(lc3::utils::PrintType::P_WARNING, true, "%d: Skipping '%s' scheduled for %d", time,
                    event->toString(state).c_str(), event->time);
                continue;
            }

            time = event->time;
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
