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
        using Callback = std::function<void(core::CallbackType, core::MachineState &)>;

        sim(utils::IPrinter & printer, utils::IInputter & inputter, uint32_t print_level);

        bool loadObjFile(std::string const & filename);
        void zeroState(void);
        void randomizeState(void);

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

        utils::IPrinter & getPrinter(void);
        utils::IPrinter const & getPrinter(void) const;
        utils::IInputter & getInputter(void);
        utils::IInputter const & getInputter(void) const;
        void setPrintLevel(uint32_t print_level);

    private:
        utils::IPrinter & printer;
        utils::IInputter & inputter;
        core::Simulator simulator;

        enum class RunType
        {
              UNTIL_INPUT
            , UNTIL_HALT
            , UNTIL_DEPTH
            , NORMAL
        } run_type;

        bool encountered_lc3_exception;
        uint64_t total_inst_exec;
        uint64_t cur_inst_exec_limit, target_inst_exec;
        uint64_t cur_sub_depth;

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
