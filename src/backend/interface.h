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
#include "simulator.h"
#include "utils.h"

namespace lc3
{
    class sim
    {
    public:
        using Callback = std::function<void(core::CallbackType, sim &)>;

        sim(utils::IPrinter & printer, utils::IInputter & inputter, uint32_t print_level);

        bool loadObjFile(std::string const & filename);
        void setup(void);
        void zeroState(void);
        uint64_t randomizeState(uint64_t seed = 0);

        void setRunInstLimit(uint64_t inst_limit);
        bool run(void);
        bool runUntilHalt(void);
        bool runUntilInputRequested(void);
        void asyncInterrupt(void);
        bool stepIn(void);
        bool stepOver(void);
        bool stepOut(void);

        uint16_t readReg(uint16_t id) const;
        uint16_t readMem(uint16_t addr) const;
        std::string getMemLine(uint16_t addr) const;
        uint16_t readPC(void) const;
        uint16_t readPSR(void) const;
        uint16_t readMCR(void) const;
        char readCC(void) const;
        void writeReg(uint16_t id, uint16_t value);
        void writeMem(uint16_t addr, uint16_t value);
        void writeStringMem(uint16_t addr, std::string const & value);
        void setMemLine(uint16_t addr, std::string const & value);
        void writePC(uint16_t value);
        void writePSR(uint16_t value);
        void writeMCR(uint16_t value);
        void writeCC(char value);

        void setBreakpoint(uint16_t addr);
        void removeBreakpoint(uint16_t addr);

        bool didExceedInstLimit(void) const;

        void registerCallback(core::CallbackType type, Callback func);

        utils::IPrinter & getPrinter(void);
        utils::IPrinter const & getPrinter(void) const;
        utils::IInputter & getInputter(void);
        utils::IInputter const & getInputter(void) const;
        void setPrintLevel(uint32_t print_level);
        void setIgnorePrivilege(bool ignore_privilege);

        uint64_t getInstExecCount(void) const;

#if (! defined API_VER) || API_VER == 1
        // Provide backward compatibility with API version.
        using callback_func_t = std::function<void(core::MachineState &)>;

        bool runUntilInputPull(void) { return runUntilInputRequested(); }

        core::MachineState & getMachineState(void);
        core::MachineState const & getMachineState(void) const;

        uint16_t getReg(uint16_t id) const { return readReg(id); }
        uint16_t getMem(uint16_t addr) const { return readMem(addr); }
        uint16_t getPC(void) const { return readPC(); }
        uint16_t getPSR(void) const { return readPSR(); }
        uint16_t getMCR(void) const { return readMCR(); }
        char getCC(void) const { return readCC(); }
        void setReg(uint16_t id, uint16_t value) { writeReg(id, value); }
        void setMem(uint16_t addr, uint16_t value) { writeMem(addr, value); }
        void setStringMem(uint16_t addr, std::string const & value) { writeStringMem(addr, value); }
        void setPC(uint16_t value) { writePC(value); }
        void setPSR(uint16_t value) { writePSR(value); }
        void setMCR(uint16_t value) { writeMCR(value); }
        void setCC(char value) { writeCC(value); }

        void registerPreInstructionCallback(callback_func_t func) {
            registerCallback(core::CallbackType::PRE_INST, [func](core::CallbackType, sim & sim_inst) {
                func(sim_inst.getMachineState());
            });
        }
        void registerPostInstructionCallback(callback_func_t func) {
            registerCallback(core::CallbackType::POST_INST, [func](core::CallbackType, sim & sim_inst) {
                func(sim_inst.getMachineState());
            });
        }
        void registerSubEnterCallback(callback_func_t func) {
            registerCallback(core::CallbackType::SUB_ENTER, [func](core::CallbackType, sim & sim_inst) {
                func(sim_inst.getMachineState());
            });
        }
        void registerSubExitCallback(callback_func_t func) {
            registerCallback(core::CallbackType::SUB_EXIT, [func](core::CallbackType, sim & sim_inst) {
                func(sim_inst.getMachineState());
            });
        }
        void registerInterruptEnterCallback(callback_func_t func) {
            registerCallback(core::CallbackType::INT_ENTER, [func](core::CallbackType, sim & sim_inst) {
                func(sim_inst.getMachineState());
            });
        }
        void registerInterruptExitCallback(callback_func_t func) {
            registerCallback(core::CallbackType::INT_EXIT, [func](core::CallbackType, sim & sim_inst) {
                func(sim_inst.getMachineState());
            });
        }
#endif

    private:
        utils::IPrinter & printer;
        utils::IInputter & inputter;
        core::Simulator simulator;

        enum class RunType
        {
              UNTIL_INPUT_REQUESTED
            , UNTIL_HALT
            , UNTIL_DEPTH
            , NORMAL
        } run_type;

        bool encountered_lc3_exception;
        uint64_t total_inst_exec;
        uint64_t cur_inst_exec_limit, target_inst_exec;
        uint64_t cur_sub_depth;

        std::unordered_map<core::CallbackType, Callback> callbacks;

        void loadOS(void);
        bool runHelper(void);
        static void callbackDispatcher(sim * sim_inst, core::CallbackType type, core::MachineState & state);
    };

    class as
    {
    public:
        as(utils::IPrinter & printer, uint32_t print_level, bool enable_liberal_asm);
        ~as(void) = default;

        optional<std::string> assemble(std::string const & asm_filename);

        void setEnableLiberalAsm(bool enable);

    private:
        utils::IPrinter & printer;
        core::Assembler assembler;
    };

    class conv
    {
    public:
        conv(utils::IPrinter & printer, uint32_t print_level);
        optional<std::string> convertBin(std::string const & asm_filename);

    private:
        utils::IPrinter & printer;
        core::Converter converter;

        bool propagate_exceptions;
    };
};

#endif
