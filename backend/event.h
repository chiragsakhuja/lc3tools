#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <string>
#include <vector>

#include "aliases.h"
#include "utils.h"

namespace lc3
{
namespace core
{
    class MachineState;

    using FutureResult = future<uint16_t>;
    using PFutureResult = std::shared_ptr<FutureResult>;

    class IEvent
    {
    public:
        uint64_t time_delta;
        PIEvent next;

        IEvent(void) : IEvent(0) {}
        IEvent(uint64_t time_delta) : time_delta(time_delta), next(nullptr), result(nullptr) {}
        virtual ~IEvent(void) = default;

        virtual void handleEvent(MachineState & state) = 0;
        virtual std::string toString(MachineState const & state) const = 0;

        void setFuture(std::shared_ptr<future<uint16_t>> result) { this->result = result; }

    protected:
        PFutureResult result;
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
    public:
        PCAddImmEvent(int16_t amnt) : PCAddImmEvent(0, amnt) {}
        PCAddImmEvent(uint64_t time_delta, int16_t amnt) :
            IEvent(time_delta), amnt(amnt) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        int16_t amnt;
    };

    class RegAddImmEvent : public IEvent
    {
    public:
        RegAddImmEvent(uint16_t reg_id, int16_t amnt) : RegAddImmEvent(0, reg_id, amnt) {}
        RegAddImmEvent(uint64_t time_delta, uint16_t reg_id, int16_t amnt) :
            IEvent(time_delta), reg_id(reg_id), amnt(amnt) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
        int16_t amnt;
    };

    class RegAddRegEvent : public IEvent
    {
    public:
        RegAddRegEvent(uint16_t reg_id1, uint16_t reg_id2) : RegAddRegEvent(0, reg_id1, reg_id2) {}
        RegAddRegEvent(uint64_t time_delta, uint16_t reg_id1, uint16_t reg_id2) :
            IEvent(time_delta), reg_id1(reg_id1), reg_id2(reg_id2) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id1, reg_id2;
    };

    class MemReadEvent : public IEvent
    {
    public:
        MemReadEvent(uint16_t reg_id, uint16_t mem_addr) : MemReadEvent(0, reg_id, mem_addr) {}
        MemReadEvent(uint64_t time_delta, uint16_t reg_id, uint16_t mem_addr) :
            IEvent(time_delta), reg_id(reg_id), mem_addr(mem_addr) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id, mem_addr;
    };

    class MemWriteImmEvent : public IEvent
    {
    public:
        MemWriteImmEvent(uint16_t addr, uint16_t value) : MemWriteImmEvent(0, addr, value) {}
        MemWriteImmEvent(uint64_t time_delta, uint16_t addr, uint16_t value) :
            IEvent(time_delta), addr(addr), value(value) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t addr, value;
    };

    class CCUpdateEvent : public IEvent
    {
    public:
        CCUpdateEvent(PFutureResult prev_result) : CCUpdateEvent(0, prev_result) {}
        CCUpdateEvent(uint64_t time_delta, PFutureResult prev_result) :
            IEvent(time_delta), prev_result(prev_result) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        PFutureResult prev_result;

        char getCCChar(uint16_t value) const;
    };
};
};

#endif
