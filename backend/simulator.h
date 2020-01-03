/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <queue>

#include "inputter.h"
#include "event.h"
#include "logger.h"
#include "printer.h"
#include "state.h"

namespace std
{
    using namespace lc3::core;

    template<> struct greater<PIEvent>
    {
        bool operator()(PIEvent lhs, PIEvent rhs)
        {
            return std::greater<uint64_t>()(lhs->time_delta, rhs->time_delta);
        }
    };
};

namespace lc3
{
namespace core
{
    class Simulator
    {
    public:
        Simulator(lc3::utils::IPrinter & printer, lc3::utils::IInputter & inputter, uint32_t print_level);
        void simulate(void);
        void loadObjFile(std::string filename, std::istream & buffer);

    private:
        std::priority_queue<PIEvent, std::vector<PIEvent>, std::greater<PIEvent>> events;
        uint64_t time;

        MachineState state;
        std::vector<PIDevice> devices;

        lc3::utils::Logger logger;
        lc3::utils::IInputter & inputter;

        void mainLoop(void);
    };
};
};

#endif

