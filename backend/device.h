#ifndef DEVICE_H
#define DEVICE_H

#include <memory>
#include <vector>

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
        IDevice(void) = default;
        virtual ~IDevice(void) {}

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const = 0;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) = 0;
        virtual std::vector<uint16_t> getAddrMap(void) const = 0;
        //virtual void tick(void) = 0;
    };

    class RWReg : public IDevice
    {
    public:
        RWReg(uint16_t data_addr) : data_addr(data_addr) { data.setValue(0x0000); }
        virtual ~RWReg(void) override = default;

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const override;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) override;
        virtual std::vector<uint16_t> getAddrMap(void) const override;

    private:
        MemLocation data;
        uint16_t data_addr;
    };

    class KeyboardDevice : public IDevice
    {
    public:
        KeyboardDevice(void) { status.setValue(0xC000); }
        virtual ~KeyboardDevice(void) override = default;

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const override;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) override;
        virtual std::vector<uint16_t> getAddrMap(void) const override;

    private:
        MemLocation status;
        MemLocation data;
    };

    using PIDevice = std::shared_ptr<IDevice>;
};
};

#endif
