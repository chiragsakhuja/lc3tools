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

        MachineState(sim & simulator, lc3::utils::Logger & logger) : logger(logger), simulator(simulator) {}

        std::vector<MemEntry> mem;
        std::array<uint32_t, 8> regs;
        uint32_t pc;

        uint32_t backup_sp;
        SPType sp_type_in_use;

        std::stack<SysCallType> sys_call_types;

        lc3::utils::Logger & logger;
        std::vector<char> console_buffer;

        bool running;
        bool hit_breakpoint;

        uint32_t readMem(uint32_t addr, bool & change_mem, std::shared_ptr<IEvent> & change) const;

        bool pre_instruction_callback_v;
        bool post_instruction_callback_v;
        bool interrupt_enter_callback_v;;
        bool interrupt_exit_callback_v;;
        bool sub_enter_callback_v;;
        bool sub_exit_callback_v;;
        callback_func_t pre_instruction_callback;
        callback_func_t post_instruction_callback;
        callback_func_t interrupt_enter_callback;
        callback_func_t interrupt_exit_callback;
        callback_func_t sub_enter_callback;
        callback_func_t sub_exit_callback;

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

        virtual void updateState(MachineState & state) const override { state.mem[PSR].setValue(value & 0xFFFF); }
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("PSR: 0x%0.4x => 0x%0.4x", state.mem[PSR].getValue(), value); }
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
            return utils::ssprintf("MEM[0x%0.4x]: 0x%0.4x => 0x%0.4x", addr, state.mem[addr].getValue(), value); }
    private:
        uint32_t addr;
        uint32_t value;
    };

    class SwapSPEvent : public IEvent
    {
    public:
        SwapSPEvent(MachineState::SPType target) : IEvent(EventType::EVENT_SWAP_SP), target(target) {}

        virtual void updateState(MachineState & state) const override;
        virtual std::string getOutputString(MachineState const & state) const override {
            return utils::ssprintf("R6 <=> SP : 0x%0.4x <=> 0x%0.4x", state.regs[6], state.backup_sp);
        }
    private:
        MachineState::SPType target;
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
