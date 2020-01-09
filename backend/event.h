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
        uint64_t time;
        PIMicroOp uops;

        IEvent(void) : IEvent(0) {}
        IEvent(uint64_t time) : time(time), uops(nullptr) {}
        virtual ~IEvent(void) = default;

        virtual void handleEvent(MachineState & state) = 0;
        virtual std::string toString(MachineState const & state) const = 0;
    };

    class AtomicInstProcessEvent : public IEvent
    {
    public:
        AtomicInstProcessEvent(sim::Decoder const & decoder) : AtomicInstProcessEvent(0, decoder) {}
        AtomicInstProcessEvent(uint64_t time, sim::Decoder const & decoder) :
            IEvent(time), decoder(decoder)
        {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        sim::Decoder const & decoder;
    };

    class StartupEvent : public IEvent
    {
    public:
        StartupEvent(uint64_t time) : IEvent(time) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class LoadObjFileEvent : public IEvent
    {
    public:
        LoadObjFileEvent(uint64_t time, std::string filename, std::istream & buffer) :
            IEvent(time), filename(filename), buffer(buffer) {}

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        std::string filename;
        std::istream & buffer;
    };

    class DeviceUpdateEvent : public IEvent
    {
    public:
        DeviceUpdateEvent(PIDevice device) : DeviceUpdateEvent(0, device) { }
        DeviceUpdateEvent(uint64_t time, PIDevice device) : IEvent(time), device(device) { }

        virtual void handleEvent(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        PIDevice device;
    };
};
};

#endif
