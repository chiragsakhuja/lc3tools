/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef STATE_H
#define STATE_H

#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include "aliases.h"
#include "callback.h"
#include "device.h"
#include "func_type.h"
#include "intex.h"
#include "mem.h"

namespace lc3
{
namespace core
{
    class IEvent;
    using PIEvent = std::shared_ptr<IEvent>;

    class MachineState
    {
    public:
        MachineState(void);

        void reinitialize(void);
        bool getIgnorePrivilege(void) const;
        void setIgnorePrivilege(bool ignore);

        uint16_t readPC(void) const { return pc; }
        void writePC(uint16_t value) { pc = value; }
        uint16_t readResetPC(void) const { return reset_pc; }
        void writeResetPC(uint16_t value) { reset_pc = value; }

        uint16_t readIR(void) const { return ir; }
        void writeIR(uint16_t value) { ir = value; }

        PIInstruction readDecodedIR(void) const { return decoded_ir; }
        void writeDecodedIR(PIInstruction value) { decoded_ir = value; }

        uint16_t readSSP(void) const { return ssp; }
        void writeSSP(uint16_t value) { ssp = value; }

        uint16_t readPSR(void) const { return std::get<0>(readMem(PSR)); }
        void writePSR(uint16_t value) { writeMem(PSR, value); }

        uint16_t readMCR(void) const { return std::get<0>(readMem(MCR)); }
        void writeMCR(uint16_t value) { writeMem(MCR, value); }

        uint16_t readReg(uint16_t id) const { return rf[id]; }
        void writeReg(uint16_t id, uint16_t value) { rf[id] = value; }

        std::pair<uint16_t, PIMicroOp> readMem(uint16_t addr) const;
        PIMicroOp writeMem(uint16_t addr, uint16_t value);
        std::string getMemLine(uint16_t addr) const;
        void setMemLine(uint16_t addr, std::string const & value);

        void registerDeviceReg(uint16_t mem_addr, PIDevice device);

        void enqueueInterrupt(InterruptType type) { pending_interrupts.push(type); }
        InterruptType peekInterrupt(void) const;
        InterruptType dequeueInterrupt(void);


        bool isFirstInit(void) const { return first_init; }
        void completeFirstInit(void) { first_init = false; }

        void pushFuncTraceType(FuncType type) { func_trace.push(type); }
        FuncType peekFuncTraceType(void) const;
        FuncType popFuncTraceType(void);

        std::vector<CallbackType> const & getPendingCallbacks(void) const { return pending_callbacks; }
        void clearPendingCallbacks(void) { pending_callbacks.clear(); }
        void addPendingCallback(CallbackType type) { pending_callbacks.push_back(type); }

    private:
        // Hardware state.
        std::vector<MemLocation> mem;
        std::vector<uint16_t> rf;
        std::unordered_map<uint16_t, PIDevice> mmio;
        uint16_t reset_pc, pc, ir;
        PIInstruction decoded_ir;
        uint16_t ssp;
        std::queue<InterruptType> pending_interrupts;

        // Simulation state.
        bool ignore_privilege;
        bool first_init;

        std::stack<FuncType> func_trace;
        std::vector<CallbackType> pending_callbacks;
    };
};
};

#endif
