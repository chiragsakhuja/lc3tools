#ifndef CORE_H
#define CORE_H

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 9
#endif

#include <functional>

#include "assembler.h"
#include "simulator.h"

namespace lc3 {
    using callback_func_t = std::function<void(core::MachineState & state)>;

    class sim
    {
    public:
        sim(utils::IPrinter & printer, utils::IInputter & inputter) : sim(printer, inputter, _PRINT_LEVEL) {}
        sim(utils::IPrinter & printer, utils::IInputter & inputter, uint32_t print_level) :
            simulator(printer, inputter, print_level)
        {
            simulator.loadOS();
        }
        ~sim(void) = default;

        void loadObjectFile(std::string const & obj_filename) { simulator.loadObjectFile(obj_filename); }
        void simulate(void) { simulator.simulate(); }
        void reset(void) { simulator.reset(); }
        void registerPreInstructionCallback(callback_func_t func) { simulator.registerPreInstructionCallback(func); }
        void registerPostInstructionCallback(callback_func_t func) { simulator.registerPostInstructionCallback(func); }
        void registerInterruptEnterCallback(callback_func_t func) { simulator.registerInterruptEnterCallback(func); }
        void registerInterruptExitCallback(callback_func_t func) { simulator.registerInterruptExitCallback(func); }
        void registerSubEnterCallback(callback_func_t func) { simulator.registerSubEnterCallback(func); }
        void registerSubExitCallback(callback_func_t func) { simulator.registerSubExitCallback(func); }
        core::MachineState & getMachineState(void) { return simulator.getMachineState(); }

    private:
        core::Simulator simulator;
    };

    class as
    {
    public:
        as(utils::IPrinter & printer) : as(printer, _PRINT_LEVEL) {}
        as(utils::IPrinter & printer, uint32_t print_level) : assembler(printer, print_level) {}
        ~as(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename) {
            assembler.assemble(asm_filename, obj_filename);
        }

    private:
        core::Assembler assembler;
    };
};

#endif
