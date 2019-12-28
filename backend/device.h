#ifndef DEVICE_H
#define DEVICE_H

#include <memory>

#include "device_regs.h"
#include "uop.h"
#include "mem_new.h"

namespace lc3
{
namespace core
{
    class IDevice
    {
    public:
        virtual ~IDevice(void) {}

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const = 0;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) = 0;
        //virtual void tick(void) = 0;
    };

    class KeyboardDevice : public IDevice
    {
    private:
        MemLocation status;
        MemLocation data;

    public:
        KeyboardDevice(void) { status.setValue(0xC000); }
        virtual ~KeyboardDevice(void) override = default;

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const override;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) override;
    };

    using PIDevice = std::shared_ptr<IDevice>;
};
};

#endif
