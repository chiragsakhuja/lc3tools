#ifndef CORE_H
#define CORE_H

#include <array>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.h"

#include "tokens.h"

#include "inputter.h"

#include "printer.h"
#include "logger.h"

#include "statement.h"

#include "state.h"

#include "instructions.h"
#include "instruction_encoder.h"
#include "instruction_decoder.h"

#include "simulator.h"
#include "assembler.h"

namespace core {
    using callback_func_t = std::function<void(MachineState & state)>;

    class lc3
    {
    public:
        lc3(utils::IPrinter & printer, utils::IInputter & inputter) : as(true, printer),
            sim(true, printer, inputter) {}
        ~lc3(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename) {
            as.assemble(asm_filename, obj_filename);
        }

        void initializeSimulator(void) { sim.loadOS(); }
        void loadSimulatorWithFile(std::string const & obj_filename) { sim.loadObjectFile(obj_filename); }
        void simulate(void) { sim.simulate(); }
        void resetSimulator(void) { sim.reset(); }
        void registerPreInstructionCallback(callback_func_t func) {
            sim.registerPreInstructionCallback(func);
        }
        void registerPostInstructionCallback(callback_func_t func) {
            sim.registerPostInstructionCallback(func);
        }
        void registerInterruptEnterCallback(callback_func_t func) {
            sim.registerInterruptEnterCallback(func);
        }
        void registerInterruptExitCallback(callback_func_t func) {
            sim.registerInterruptExitCallback(func);
        }
        void registerSubEnterCallback(callback_func_t func) {
            sim.registerSubEnterCallback(func);
        }
        void registerSubExitCallback(callback_func_t func) {
            sim.registerSubExitCallback(func);
        }
        MachineState & getMachineState(void) { return sim.getMachineState(); }

    private:
        Assembler as;
        Simulator sim;
    };
};

#endif
