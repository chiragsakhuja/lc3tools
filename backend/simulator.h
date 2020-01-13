/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <unordered_map>
#include <queue>
#include <set>

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
            return std::greater<uint64_t>()(lhs->time, rhs->time);
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
        using Callback = std::function<void(MachineState &)>;

        Simulator(lc3::utils::IPrinter & printer, lc3::utils::IInputter & inputter, uint32_t print_level);
        void simulate(void);
        void loadObjFile(std::string filename, std::istream & buffer);
        void registerCallback(CallbackType type, Callback func);
        void addBreakpoint(uint16_t pc);
        void removeBreakpoint(uint16_t pc);

    private:
        std::priority_queue<PIEvent, std::vector<PIEvent>, std::greater<PIEvent>> events;
        uint64_t time;

        MachineState state;
        std::vector<PIDevice> devices;

        lc3::utils::Logger logger;
        lc3::utils::IInputter & inputter;

        std::unordered_map<CallbackType, Callback> callbacks;
        std::set<uint16_t> breakpoints;

        void mainLoop(void);

        friend void callbackDispatcher(Simulator *, CallbackType, MachineState &);
        static void callbackDispatcher(Simulator * sim, CallbackType type, MachineState & state);
    };
};
};

#endif

