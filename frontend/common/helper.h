#ifndef HELPER_H
#define HELPER_H

#include "interface.h"

namespace lc3
{
    struct Breakpoint { uint32_t id, loc; };

    extern core::lc3 * interface;
    extern uint32_t inst_exec_count;
    extern std::vector<Breakpoint> breakpoints;

    namespace as
    {
        std::string assemble(std::string const & asm_filename);
    };

    namespace sim
    {
        void init(utils::IPrinter & printer, utils::IInputter & inputter);
        void randomizeMachine(void);
        void shutdown(void);
        void restart(void);
        bool loadSimulatorWithFile(std::string const & filename);
        void setRunInstLimit(uint32_t inst_count);
        bool run(void);
        bool runUntilHalt(void);
        bool stepOver(void);
        bool stepOut(void);
        void registerPreInstructionCallback(core::callback_func_t func);
        void registerPostInstructionCallback(core::callback_func_t func);
        void registerInterruptEnterCallback(core::callback_func_t func);
        void registerInterruptExitCallback(core::callback_func_t func);
        void registerSubEnterCallback(core::callback_func_t func);
        void registerSubExitCallback(core::callback_func_t func);
        void registerBreakpointHitCallback(std::function<void(core::MachineState & state, Breakpoint const & bp)> func);

        uint32_t simGetReg(uint32_t id);
        uint32_t simGetPC(void);
        uint32_t simGetPSR(void);
        char simGetCC(void);
        uint32_t simGetMemVal(uint32_t addr);
        std::string simGetMemLine(uint32_t addr);
        void simSetReg(uint32_t id, uint32_t value);
        void simSetPC(uint32_t value);
        void simSetPSR(uint32_t value);
        void simSetCC(char cc);
        void simSetMemVal(uint32_t addr, uint32_t value);

        Breakpoint simSetBreakpoint(uint32_t addr);
        bool simRemoveBreakpoint(uint32_t id);
    };
};

#endif
