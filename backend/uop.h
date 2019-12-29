#ifndef UOP_H
#define UOP_H

#include <memory>

#include "aliases.h"
#include "utils.h"

namespace lc3
{
namespace core
{
    namespace sim { class Decoder; };
    class MachineState;

    class IMicroOp
    {
    public:
        IMicroOp(void) : next(nullptr) { }
        virtual ~IMicroOp(void) = default;

        virtual void handleMicroOp(MachineState & state) = 0;
        virtual std::string toString(MachineState const & state) const = 0;
        PIMicroOp insert(PIMicroOp new_next);
        PIMicroOp getNext(void) const { return next; }

    protected:
        PIMicroOp next;

        std::string regToString(uint16_t reg_id) const;
    };

    class FetchMicroOp : public IMicroOp
    {
    public:
        FetchMicroOp(void) = default;

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class DecodeMicroOp : public IMicroOp
    {
    public:
        DecodeMicroOp(sim::Decoder const & decoder) : IMicroOp(), decoder(decoder) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        sim::Decoder const & decoder;
    };

    class PCWriteRegMicroOp : public IMicroOp
    {
    public:
        PCWriteRegMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
    };

    class PSRWriteRegMicroOp : public IMicroOp
    {
    public:
        PSRWriteRegMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
    };

    class PCAddImmMicroOp : public IMicroOp
    {
    public:
        PCAddImmMicroOp(int16_t amnt) : IMicroOp(), amnt(amnt) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t amnt;
    };

    class RegWriteImmMicroOp : public IMicroOp
    {
    public:
        RegWriteImmMicroOp(uint16_t reg_id, uint16_t value) : IMicroOp(), reg_id(reg_id), value(value) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id, value;
    };

    class RegWriteRegMicroOp : public IMicroOp
    {
    public:
        RegWriteRegMicroOp(uint16_t dst_id, uint16_t src_id) : IMicroOp(), dst_id(dst_id), src_id(src_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, src_id;
    };

    class RegWritePCMicroOp : public IMicroOp
    {
    public:
        RegWritePCMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
    };

    class RegWritePSRMicroOp : public IMicroOp
    {
    public:
        RegWritePSRMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
    };

    class RegWriteSSPMicroOp : public IMicroOp
    {
    public:
        RegWriteSSPMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
    };

    class SSPWriteRegMicroOp : public IMicroOp
    {
    public:
        SSPWriteRegMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
    };

    class RegAddImmMicroOp : public IMicroOp
    {
    public:
        RegAddImmMicroOp(uint16_t dst_id, uint16_t src_id, uint16_t amnt) :
            IMicroOp(), dst_id(dst_id), src_id(src_id), amnt(amnt) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, src_id;
        uint16_t amnt;
    };

    class RegAddRegMicroOp : public IMicroOp
    {
    public:
        RegAddRegMicroOp(uint16_t dst_id, uint16_t src_id1, uint16_t src_id2) :
            IMicroOp(), dst_id(dst_id), src_id1(src_id1), src_id2(src_id2) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, src_id1, src_id2;
    };

    class RegAndImmMicroOp : public IMicroOp
    {
    public:
        RegAndImmMicroOp(uint16_t dst_id, uint16_t src_id, uint16_t amnt) :
            IMicroOp(), dst_id(dst_id), src_id(src_id), amnt(amnt) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, src_id;
        uint16_t amnt;
    };

    class RegAndRegMicroOp : public IMicroOp
    {
    public:
        RegAndRegMicroOp(uint16_t dst_id, uint16_t src_id1, uint16_t src_id2) :
            IMicroOp(), dst_id(dst_id), src_id1(src_id1), src_id2(src_id2) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, src_id1, src_id2;
    };

    class RegNotMicroOp : public IMicroOp
    {
    public:
        RegNotMicroOp(uint16_t dst_id, uint16_t src_id) :
            IMicroOp(), dst_id(dst_id), src_id(src_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, src_id;
    };

    class MemReadMicroOp : public IMicroOp
    {
    public:
        MemReadMicroOp(uint16_t dst_id, uint16_t addr_reg_id) :
            IMicroOp(), dst_id(dst_id), addr_reg_id(addr_reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t dst_id, addr_reg_id;
    };

    class MemWriteImmMicroOp : public IMicroOp
    {
    public:
        MemWriteImmMicroOp(uint16_t addr_reg_id, uint16_t value) :
            IMicroOp(), addr_reg_id(addr_reg_id), value(value) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t addr_reg_id, value;
    };

    class MemWriteRegMicroOp : public IMicroOp
    {
    public:
        MemWriteRegMicroOp (uint16_t addr_reg_id, uint16_t src_id) :
            IMicroOp(), addr_reg_id(addr_reg_id), src_id(src_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t addr_reg_id, src_id;
    };

    class CCUpdateRegMicroOp : public IMicroOp
    {
    public:
        CCUpdateRegMicroOp(uint16_t reg_id) : IMicroOp(), reg_id(reg_id) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;

        char getCCChar(uint16_t value) const;
    };
};
};

#endif
