/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef CORE_H
#define CORE_H

#ifndef DEFAULT_PRINT_LEVEL
    #ifdef _ENABLE_DEBUG
        #define DEFAULT_PRINT_LEVEL 9
    #else
        #define DEFAULT_PRINT_LEVEL 4
    #endif
#endif

#include <functional>
#include <utility>

#include "assembler.h"
#include "converter.h"
#include "optional.h"
#include "simulator.h"

namespace lc3
{
    class sim;

    struct Breakpoint
    {
        // TODO: make this moveable and replace sim_inst with a reference
        Breakpoint(uint32_t id, uint32_t loc, sim * sim_int) : id(id), loc(loc), sim_int(sim_int) {}

        uint32_t id, loc;
        sim const * sim_int;
    };

    using callback_func_t = std::function<void(core::MachineState &)>;
    using breakpoint_callback_func_t = std::function<void(core::MachineState & state, Breakpoint const & bp)>;

    class sim
    {
    public:
        sim(utils::IPrinter & printer, utils::IInputter & inputter, bool threaded_input,
            uint32_t print_level = DEFAULT_PRINT_LEVEL, bool propagate_exceptions = false);
        ~sim(void) = default;

        bool loadObjFile(std::string const & obj_filename);
        void reinitialize(void);
        void randomize(void);
        void restart(void);

        void setRunInstLimit(uint64_t inst_limit);
        bool run(void);
        bool runUntilHalt(void);
        bool runUntilInputPoll(void);
        void pause(void);
        bool stepIn(void);
        bool stepOver(void);
        bool stepOut(void);

        core::MachineState & getMachineState(void);
        core::MachineState const & getMachineState(void) const;
        uint64_t getInstExecCount(void) const;
        bool didExceedInstLimit(void) const;
        std::vector<Breakpoint> const & getBreakpoints() const;

        uint16_t getReg(uint16_t id) const;
        uint16_t getMem(uint16_t addr) const;
        std::string getMemLine(uint16_t addr) const;
        uint16_t getPC(void) const;
        uint16_t getPSR(void) const;
        uint16_t getMCR(void) const;
        char getCC(void) const;
        void setReg(uint16_t id, uint16_t value);
        void setMem(uint16_t addr, uint16_t value);
        void setMemString(uint16_t addr, std::string const & value);
        void setMemLine(uint16_t addr, std::string const & value);
        void setPC(uint16_t value);
        void setPSR(uint16_t value);
        void setMCR(uint16_t value);
        void setCC(char value);

        Breakpoint setBreakpoint(uint16_t addr);
        bool removeBreakpointByID(uint32_t id);
        bool removeBreakpointByAddr(uint16_t addr);

        void registerPreInstructionCallback(callback_func_t func);
        void registerPostInstructionCallback(callback_func_t func);
        void registerInterruptEnterCallback(callback_func_t func);
        void registerInterruptExitCallback(callback_func_t func);
        void registerSubEnterCallback(callback_func_t func);
        void registerSubExitCallback(callback_func_t func);
        void registerWaitForInputCallback(callback_func_t func);
        void registerBreakpointCallback(breakpoint_callback_func_t func);

        utils::IPrinter & getPrinter(void);
        utils::IPrinter const & getPrinter(void) const;
        void setPrintLevel(uint32_t print_level);
        void setPropagateExceptions(void);
        void clearPropagateExceptions(void);

    private:
        utils::IPrinter & printer;
        core::Simulator simulator;

        friend class core::Simulator;
        static void preInstructionCallback(sim & sim_int, core::MachineState & state);
        static void postInstructionCallback(sim & sim_int, core::MachineState & state);
        static void interruptEnterCallback(sim & sim_int, core::MachineState & state);
        static void interruptExitCallback(sim & sim_int, core::MachineState & state);
        static void subEnterCallback(sim & sim_int, core::MachineState & state);
        static void subExitCallback(sim & sim_int, core::MachineState & state);
        static void waitForInputCallback(sim & sim_int, core::MachineState & state);

        uint64_t inst_exec_count = 0;
        uint64_t total_inst_limit = 0;
        uint64_t inst_limit = 0;
        int64_t remaining_inst_count = -1;
        int32_t sub_depth = 0;

        bool pre_instruction_callback_v = false;
        bool post_instruction_callback_v = false;
        bool interrupt_enter_callback_v = false;
        bool interrupt_exit_callback_v = false;
        bool sub_enter_callback_v = false;
        bool sub_exit_callback_v = false;
        bool wait_for_input_callback_v = false;
        bool breakpoint_callback_v = false;
        callback_func_t pre_instruction_callback;
        callback_func_t post_instruction_callback;
        callback_func_t interrupt_enter_callback;
        callback_func_t interrupt_exit_callback;
        callback_func_t sub_enter_callback;
        callback_func_t sub_exit_callback;
        callback_func_t wait_for_input_callback;
        breakpoint_callback_func_t breakpoint_callback;

        uint32_t breakpoint_id = 0;
        std::vector<Breakpoint> breakpoints;

        bool propagate_exceptions;

        enum class RunType
        {
              UNTIL_INPUT
            , UNTIL_HALT
            , UNTIL_DEPTH
            , NORMAL
        } run_type;

        void loadOS(void);
        bool run(RunType cur_run_type);
    };

    class as
    {
    public:
        as(utils::IPrinter & printer, uint32_t print_level = DEFAULT_PRINT_LEVEL, bool propagate_exceptions = false) :
            printer(printer), assembler(printer, print_level), propagate_exceptions(propagate_exceptions) {}
        ~as(void) = default;

        optional<std::string> assemble(std::string const & asm_filename);

        void setPropagateExceptions(void);
        void clearPropagateExceptions(void);

    private:
        friend class sim;

        utils::IPrinter & printer;
        core::Assembler assembler;

        bool propagate_exceptions;
    };

    class conv
    {
    public:
        conv(utils::IPrinter & printer, uint32_t print_level = DEFAULT_PRINT_LEVEL, bool propagate_exceptions = false) :
            printer(printer), converter(printer, print_level), propagate_exceptions(propagate_exceptions) {}
        optional<std::string> convertBin(std::string const & asm_filename);

        void setPropagateExceptions(void);
        void clearPropagateExceptions(void);

    private:
        utils::IPrinter & printer;
        core::Converter converter;

        bool propagate_exceptions;
    };
};

#endif
