#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <string>
#include <vector>

namespace lc3
{
namespace core
{
    class MachineState;
    class IEvent;
    using PIEvent = std::shared_ptr<IEvent>;

    class IEvent
    {
    public:
        uint64_t time_delta;
        PIEvent next;

        IEvent(void) : IEvent(0) {}
        IEvent(uint64_t time_delta) : time_delta(time_delta), next(nullptr) {}
        virtual ~IEvent(void) = default;

        virtual void handleEvent(MachineState & state) = 0;
        virtual std::string toString(MachineState const & state) const = 0;
    };

    class AtomicInstProcessEvent : public IEvent
    {
    public:
        AtomicInstProcessEvent(void) : AtomicInstProcessEvent(0) {}
        AtomicInstProcessEvent(uint64_t time_delta) : IEvent(time_delta) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class FetchEvent : public IEvent
    {
    public:
        FetchEvent(void) : FetchEvent(0) {}
        FetchEvent(uint64_t time_delta) : IEvent(time_delta) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class PCAddImmEvent : public IEvent
    {
    private:
        int16_t amnt;

    public:
        PCAddImmEvent(int16_t amnt) : PCAddImmEvent(0, amnt) {}
        PCAddImmEvent(uint64_t time_delta, int16_t amnt) :
            IEvent(time_delta), amnt(amnt) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class MemReadEvent : public IEvent
    {
    private:
        uint16_t reg_id, mem_addr;
    public:
        MemReadEvent(uint16_t reg_id, uint16_t mem_addr) : MemReadEvent(0, reg_id, mem_addr) {}
        MemReadEvent(uint64_t time_delta, uint16_t reg_id, uint16_t mem_addr) :
            IEvent(time_delta), reg_id(reg_id), mem_addr(mem_addr) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class MemWriteImmEvent : public IEvent
    {
    private:
        uint16_t addr, value;
    public:
        MemWriteImmEvent(uint16_t addr, uint16_t value) : MemWriteImmEvent(0, addr, value) {}
        MemWriteImmEvent(uint64_t time_delta, uint16_t addr, uint16_t value) :
            IEvent(time_delta), addr(addr), value(value) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };
};
};

#endif
