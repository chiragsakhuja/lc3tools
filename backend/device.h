#ifndef DEVICE_H
#define DEVICE_H

#include <memory>
#include <vector>

#include "device_regs.h"
#include "inputter.h"
#include "intex.h"
#include "logger.h"
#include "mem.h"
#include "uop.h"

namespace lc3
{
namespace core
{
    class IDevice
    {
    public:
        IDevice(void) = default;
        virtual ~IDevice(void) {}

        virtual void startup(void) { }
        virtual void shutdown(void) { }
        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const = 0;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) = 0;
        virtual std::vector<uint16_t> getAddrMap(void) const = 0;
        virtual std::string getName(void) const = 0;
        virtual PIMicroOp tick(void) { return nullptr; }
    };

    class RWReg : public IDevice
    {
    public:
        RWReg(uint16_t data_addr) : data_addr(data_addr) { data.setValue(0x0000); }
        virtual ~RWReg(void) override = default;

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const override;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) override;
        virtual std::vector<uint16_t> getAddrMap(void) const override;
        virtual std::string getName(void) const override { return "RWReg"; }

    private:
        MemLocation data;
        uint16_t data_addr;
    };

    class KeyboardDevice : public IDevice
    {
    public:
        KeyboardDevice(lc3::utils::IInputter & inputter);
        virtual ~KeyboardDevice(void) override = default;

        virtual void startup(void) override;
        virtual void shutdown(void) override;
        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const override;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) override;
        virtual std::vector<uint16_t> getAddrMap(void) const override;
        virtual std::string getName(void) const override { return "Keyboard"; }
        virtual PIMicroOp tick(void) override;

    private:
        lc3::utils::IInputter & inputter;

        MemLocation status;
        MemLocation data;
        bool interrupt_triggered;
    };

    class DisplayDevice : public IDevice
    {
    public:
        DisplayDevice(lc3::utils::Logger & logger);
        virtual ~DisplayDevice(void) override = default;

        virtual std::pair<uint16_t, PIMicroOp> read(uint16_t addr) const override;
        virtual PIMicroOp write(uint16_t addr, uint16_t value) override;
        virtual std::vector<uint16_t> getAddrMap(void) const override;
        virtual std::string getName(void) const override { return "Display"; }
        virtual PIMicroOp tick(void) override;

    private:
        lc3::utils::Logger & logger;

        MemLocation status;
        MemLocation data;
    };
};
};

#endif
