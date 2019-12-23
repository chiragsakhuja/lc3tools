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

        virtual void handleEvent(MachineState & state) const = 0;
        virtual std::string toString(MachineState const & state) const = 0;
    };

    class FetchEvent : public IEvent
    {
    public:
        FetchEvent(void) : FetchEvent(0) {}
        FetchEvent(uint64_t time_delta) : IEvent(time_delta) {}

        virtual void handleEvent(MachineState & state) const override;
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

        virtual void handleEvent(MachineState & state) const override;
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

        virtual void handleEvent(MachineState & state) const override;
        virtual std::string toString(MachineState const & state) const override;
    };
};
};

#endif
