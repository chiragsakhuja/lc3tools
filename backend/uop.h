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

    using FutureResult = future<uint16_t>;
    using PFutureResult = std::shared_ptr<FutureResult>;

    class IMicroOp
    {
    public:
        IMicroOp(void) : IMicroOp(nullptr) {}
        IMicroOp(PFutureResult result) : result(result), next(nullptr) {}
        virtual ~IMicroOp(void) = default;

        virtual void handleMicroOp(MachineState & state) = 0;
        virtual std::string toString(MachineState const & state) const = 0;
        PIMicroOp insert(PIMicroOp new_next);
        PIMicroOp getNext(void) const { return next; }

    protected:
        PFutureResult result;
        PIMicroOp next;
    };

    class FetchMicroOp : public IMicroOp
    {
    public:
        FetchMicroOp(void) : FetchMicroOp(nullptr) {}
        FetchMicroOp(PFutureResult result) : IMicroOp(result) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class DecodeMicroOp : public IMicroOp
    {
    public:
        DecodeMicroOp(sim::Decoder const & decoder) : IMicroOp(nullptr), decoder(decoder) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        sim::Decoder const & decoder;
    };

    class PCAddImmMicroOp : public IMicroOp
    {
    public:
        PCAddImmMicroOp(int16_t amnt) : PCAddImmMicroOp(nullptr, amnt) {}
        PCAddImmMicroOp(PFutureResult result, int16_t amnt) :
            IMicroOp(result), amnt(amnt) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        int16_t amnt;
    };

    class RegAddImmMicroOp : public IMicroOp
    {
    public:
        RegAddImmMicroOp(uint16_t reg_id, int16_t amnt) : RegAddImmMicroOp(nullptr, reg_id, amnt) {}
        RegAddImmMicroOp(PFutureResult result, uint16_t reg_id, int16_t amnt) :
            IMicroOp(result), reg_id(reg_id), amnt(amnt) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id;
        int16_t amnt;
    };

    class RegAddRegMicroOp : public IMicroOp
    {
    public:
        RegAddRegMicroOp(uint16_t reg_id1, uint16_t reg_id2) : RegAddRegMicroOp(nullptr, reg_id1, reg_id2) {}
        RegAddRegMicroOp(PFutureResult result, uint16_t reg_id1, uint16_t reg_id2) :
            IMicroOp(result), reg_id1(reg_id1), reg_id2(reg_id2) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id1, reg_id2;
    };

    class MemReadMicroOp : public IMicroOp
    {
    public:
        MemReadMicroOp(uint16_t reg_id, uint16_t mem_addr) : MemReadMicroOp(nullptr, reg_id, mem_addr) {}
        MemReadMicroOp(PFutureResult result, uint16_t reg_id, uint16_t mem_addr) :
            IMicroOp(result), reg_id(reg_id), mem_addr(mem_addr) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t reg_id, mem_addr;
    };

    class MemWriteImmMicroOp : public IMicroOp
    {
    public:
        MemWriteImmMicroOp(uint16_t addr, uint16_t value) : IMicroOp(nullptr), addr(addr), value(value) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        uint16_t addr, value;
    };

    class CCUpdateMicroOp : public IMicroOp
    {
    public:
        CCUpdateMicroOp(PFutureResult prev_result) : IMicroOp(nullptr), prev_result(prev_result) {}

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        PFutureResult prev_result;

        char getCCChar(uint16_t value) const;
    };
};
};

#endif
