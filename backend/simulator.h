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
            uint32_t print_level);
        ~Simulator(void) = default;

        void loadObjectFile(std::string const & obj_file);
        void simulate(void);
        void pause(void);
        void reinitialize(void);

        void registerPreInstructionCallback(callback_func_t func);
        void registerPostInstructionCallback(callback_func_t func);
        void registerInterruptEnterCallback(callback_func_t func);
        void registerInterruptExitCallback(callback_func_t func);
        void registerSubEnterCallback(callback_func_t func);
        void registerSubExitCallback(callback_func_t func);
        void registerInputPollCallback(callback_func_t func);

        MachineState & getMachineState(void) { return state; }
        MachineState const & getMachineState(void) const { return state; }

        void setPrintLevel(uint32_t print_level) { logger.setPrintLevel(print_level); }
        uint32_t getPrintLevel(void) const { return logger.getPrintLevel(); }

        void loadObjectFileFromBuffer(std::istream & buffer);

    private:
        sim::InstructionDecoder decoder;

        MachineState state;

        lc3::utils::Logger logger;
        lc3::utils::IInputter & inputter;

        std::atomic<bool> collecting_input;

        std::vector<PIEvent> executeInstruction(void);
        std::vector<PIEvent> checkAndSetupInterrupts();
        void executeEventChain(std::vector<PIEvent> & events);
        void executeEvent(PIEvent event);
        void updateDevices(void);
        void handleInput(void);
    };
};
};

#endif
