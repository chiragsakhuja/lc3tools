/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef STATE_H
#define STATE_H

#include <array>
#include <functional>
#include <memory>
#include <stack>
#include <vector>

#include "device_regs.h"
#include "logger.h"
#include "mem.h"

namespace lc3
{
class sim;

namespace core
{
    class IEvent;
    struct MachineState;

    using callback_func_t = std::function<void(sim &, MachineState &)>;

    struct MachineState
    {
        enum class SysCallType {
              TRAP
            , INT
        };

        enum class SPType {
              SSP
            , USP
        };

        MachineState(sim & simulator, lc3::utils::Logger & logger) : pc(0), pre_instruction_callback_v(false),
			post_instruction_callback_v(false), interrupt_enter_callback_v(false),
			interrupt_exit_callback_v(false), sub_enter_callback_v(false),
			sub_exit_callback_v(false), wait_for_input_callback_v(false),
			logger(logger), simulator(simulator) {}

        std::vector<MemEntry> mem;
        std::array<uint32_t, 8> regs;
        uint32_t pc;

        std::stack<SysCallType> sys_call_types;

        lc3::utils::Logger & logger;

        uint32_t readMemEvent(uint32_t addr, bool & change_mem, std::shared_ptr<IEvent> & change) const;
        uint32_t readMemSafe(uint32_t addr);
        uint32_t readMemRaw(uint32_t addr) const;
        void writeMemEvent(uint32_t addr, uint16_t value, bool & change_mem, std::shared_ptr<IEvent> & change);
        void writeMemSafe(uint32_t addr, uint16_t value);
        void writeMemRaw(uint32_t addr, uint16_t value);

        bool pre_instruction_callback_v;
        bool post_instruction_callback_v;
        bool interrupt_enter_callback_v;
        bool interrupt_exit_callback_v;
        bool sub_enter_callback_v;
        bool sub_exit_callback_v;
        bool wait_for_input_callback_v;
        callback_func_t pre_instruction_callback;
        callback_func_t post_instruction_callback;
        callback_func_t interrupt_enter_callback;
        callback_func_t interrupt_exit_callback;
        callback_func_t sub_enter_callback;
        callback_func_t sub_exit_callback;
        callback_func_t wait_for_input_callback;

        sim & simulator;
    };

    enum class EventType {
          EVENT_REG
        , EVENT_PSR
        , EVENT_PC
        , EVENT_MEM
        , EVENT_SWAP_SP
        , EVENT_CALLBACK
        , PUSH_SYS_CALL_TYPE
        , POP_SYS_CALL_TYPE
    };

    class IEvent
    {
    public:
        EventType type;

        IEvent(EventType type) : type(type) {}
        virtual ~IEvent(void) = default;

        virtual void updateState(MachineState & state) const = 0;
        virtual std::string getOutputString(MachineState const & state) const = 0;
    };

    class RegEvent : public IEvent
    {
    public:
        RegEvent(uint32_t reg, uint32_t value) : IEvent(EventType::EVENT_REG), reg(reg), value(value) {}

        virtual void updateState(MachineState & state) const override { state.regs[reg] = value & 0xFFFF; }
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("R%d: 0x%0.4x => 0x%0.4x", reg, state.regs[reg], value); }
    private:
        uint32_t reg;
        uint32_t value;
    };

    class PSREvent : public IEvent
    {
    public:
        PSREvent(uint32_t value) : IEvent(EventType::EVENT_PSR), value(value) {}

        virtual void updateState(MachineState & state) const override { state.writeMemRaw(PSR, value & 0xFFFF); }
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("PSR: 0x%0.4x => 0x%0.4x", state.readMemRaw(PSR), value); }
    private:
        uint32_t value;
    };

    class PCEvent : public IEvent
    {
    public:
        PCEvent(uint32_t value) : IEvent(EventType::EVENT_PC), value(value) {}

        virtual void updateState(MachineState & state) const override { state.pc = value & 0xFFFF; }
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("PC: 0x%0.4x => 0x%0.4x", state.pc, value); }
    private:
        uint32_t value;
    };

    class MemWriteEvent : public IEvent
    {
    public:
        MemWriteEvent(uint32_t addr, uint32_t value) : IEvent(EventType::EVENT_MEM), addr(addr), value(value) {}

        virtual void updateState(MachineState & state) const override;
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("MEM[0x%0.4x]: 0x%0.4x => 0x%0.4x", addr, state.readMemRaw(addr), value); }
    private:
        uint32_t addr;
        uint32_t value;
    };

    class SwapSPEvent : public IEvent
    {
    public:
        SwapSPEvent() : IEvent(EventType::EVENT_SWAP_SP) {}

        virtual void updateState(MachineState & state) const override;
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("R6 <=> SP : 0x%0.4x <=> 0x%0.4x", state.regs[6], state.readMemRaw(BSP));
        }
    };

    class CallbackEvent : public IEvent
    {
    public:
        CallbackEvent(bool callback_v, callback_func_t callback) :
            IEvent(EventType::EVENT_CALLBACK), callback_v(callback_v), callback(callback) {}
        virtual void updateState(MachineState & state) const override {
            if(callback_v) { callback(state.simulator, state); }
        }
        virtual std::string getOutputString(MachineState const & state) const override { (void)state; return ""; }
    private:
        bool callback_v;
        callback_func_t callback;
    };

    class PushSysCallTypeEvent : public IEvent
    {
    public:
        PushSysCallTypeEvent(MachineState::SysCallType call_type) : IEvent(EventType::PUSH_SYS_CALL_TYPE),
            call_type(call_type) {}
        virtual void updateState(MachineState & state) const override {
            state.sys_call_types.push(call_type);
        }
        virtual std::string getOutputString(MachineState const & state) const override { (void)state; return ""; }
    private:
        MachineState::SysCallType call_type;
    };

    class PopSysCallTypeEvent : public IEvent
    {
    public:
        PopSysCallTypeEvent() : IEvent(EventType::POP_SYS_CALL_TYPE) {}
        virtual void updateState(MachineState & state) const override {
            state.sys_call_types.pop();
        }
        virtual std::string getOutputString(MachineState const & state) const override { (void)state; return ""; }
    };
};
};

#endif
