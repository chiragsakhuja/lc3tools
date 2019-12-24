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
};
};

#endif
