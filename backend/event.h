#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <string>
#include <vector>

#include "aliases.h"
#include "decoder.h"
#include "utils.h"

namespace lc3
{
namespace core
{
    class MachineState;

    class IEvent
    {
    public:
        uint64_t time_delta;
        PIMicroOp uops;

        IEvent(void) : IEvent(0) {}
        IEvent(uint64_t time_delta) : time_delta(time_delta), uops(nullptr) {}
        virtual ~IEvent(void) = default;

        virtual void handleEvent(MachineState & state) = 0;
        virtual std::string toString(MachineState const & state) const = 0;
    };

    class AtomicInstProcessEvent : public IEvent
    {
    public:
        AtomicInstProcessEvent(sim::Decoder const & decoder) : AtomicInstProcessEvent(0, decoder) {}
        AtomicInstProcessEvent(uint64_t time_delta, sim::Decoder const & decoder) :
            IEvent(time_delta), decoder(decoder)
        {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        sim::Decoder const & decoder;
    };

    class LoadObjFileEvent : public IEvent
    {
    public:
        LoadObjFileEvent(std::string filename, std::istream & buffer) : IEvent(1), filename(filename), buffer(buffer) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        std::string filename;
        std::istream & buffer;
    };
};
};

#endif
