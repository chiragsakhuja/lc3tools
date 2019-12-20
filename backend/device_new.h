#ifndef DEV_NEW_H
#define DEV_NEW_H

#include <memory>

#include "device_regs.h"
#include "events_new.h"
#include "mem_new.h"

namespace lc3
{
namespace core
{
    class IDevice
    {
    public:
        virtual ~IDevice(void) = 0;

        virtual PIEvent read(uint16_t reg_id, uint16_t mem_addr) = 0;
        virtual void write(uint16_t addr, uint16_t value) = 0;
        virtual void tick(void) = 0;

        virtual uint16_t getValue(uint16_t addr) const = 0;
        virtual void setValue(uint16_t addr, uint16_t value) = 0;
    };

    class KeyboardDevice : public IDevice
    {
    private:
        MemLocation status;
        MemLocation data;

    public:
        KeyboardDevice(void) = default;
        virtual ~KeyboardDevice(void) override = default;

        virtual PIEvent read(uint16_t reg_id, uint16_t mem_addr) override;
        virtual uint16_t getValue(uint16_t addr) const override;
        virtual void setValue(uint16_t addr, uint16_t value) override;
    };

    using PIDevice = std::shared_ptr<IDevice>;
};
};

#endif
