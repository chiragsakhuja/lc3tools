#ifndef CORE_H
#define CORE_H

#include <functional>

#include "assembler.h"
#include "simulator.h"

namespace lc3 {
    using callback_func_t = std::function<void(core::MachineState & state)>;

    class sim
    {
    public:
        sim(utils::IPrinter & printer, utils::IInputter & inputter) : simulator(true, printer, inputter) {
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
        as(utils::IPrinter & printer) : assembler(true, printer) {}
        ~as(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename) {
            assembler.assemble(asm_filename, obj_filename);
        }

    private:
        core::Assembler assembler;
    };
};

#endif
