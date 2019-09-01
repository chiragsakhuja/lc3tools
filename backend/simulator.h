/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef SIM_H
#define SIM_H

#include <atomic>
#include <functional>

#include "assembler.h"
#include "inputter.h"
#include "instruction_decoder.h"
#include "logger.h"
#include "printer.h"
#include "state.h"

namespace lc3
{
class sim;

namespace core
{
    class Simulator
    {
    public:
        Simulator(lc3::sim & simulator, lc3::utils::IPrinter & printer, utils::IInputter & inputter,
            uint32_t print_level, bool threaded_input);
        ~Simulator(void) = default;

        void loadObj(std::istream & buffer);
        void simulate(void);
        void enableClock(void);
        void disableClock(void);
        bool isClockEnabled(void) const;
        void reinitialize(void);

        void registerPreInstructionCallback(callback_func_t func);
        void registerPostInstructionCallback(callback_func_t func);
        void registerInterruptEnterCallback(callback_func_t func);
        void registerInterruptExitCallback(callback_func_t func);
        void registerSubEnterCallback(callback_func_t func);
        void registerSubExitCallback(callback_func_t func);
        void registerWaitForInputCallback(callback_func_t func);

        MachineState & getMachineState(void) { return state; }
        MachineState const & getMachineState(void) const { return state; }

        void setPrintLevel(uint32_t print_level) { logger.setPrintLevel(print_level); }
        uint32_t getPrintLevel(void) const { return logger.getPrintLevel(); }

        void setIgnorePrivilege(bool ignore);

    private:
        sim::InstructionDecoder decoder;

        MachineState state;

        lc3::utils::Logger logger;
        lc3::utils::IInputter & inputter;

        bool threaded_input;
        std::atomic<bool> collecting_input;

        std::vector<PIEvent> executeInstruction(void);
        void checkAndSetupInterrupts();
        void executeEventChain(std::vector<PIEvent> & events);
        void executeEvent(PIEvent event);
        void updateDevices(void);
        void collectInput(void);
        void inputThread(void);
    };
};
};

#endif
