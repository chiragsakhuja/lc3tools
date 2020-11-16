/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef UOP_H
#define UOP_H

#include <functional>
#include <memory>

#include "aliases.h"
#include "callback.h"
#include "func_type.h"
#include "intex.h"
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
        virtual PIMicroOp insert(PIMicroOp new_next);
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

    class BranchMicroOp : public IMicroOp
    {
    public:
        using PredFunction = std::function<bool(MachineState const & state)>;

        BranchMicroOp(PredFunction pred, std::string msg, PIMicroOp true_next, PIMicroOp false_next) :
            pred(pred), msg(msg), true_next(true_next), false_next(false_next) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
        virtual PIMicroOp insert(PIMicroOp new_next) override;

    private:
        PredFunction pred;
        std::string msg;
        PIMicroOp true_next, false_next;
    };

    class CallbackMicroOp : public IMicroOp
    {
    public:
        CallbackMicroOp(CallbackType type) : IMicroOp(), type(type) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        CallbackType type;
    };

    class PushInterruptTypeMicroOp : public IMicroOp
    {
    public:
        PushInterruptTypeMicroOp(InterruptType type) : IMicroOp(), type(type) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        InterruptType type;
    };

    class PopInterruptTypeMicroOp : public IMicroOp
    {
    public:
        PopInterruptTypeMicroOp(void) : IMicroOp() { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    class PushFuncTypeMicroOp : public IMicroOp
    {
    public:
        PushFuncTypeMicroOp(FuncType type) : IMicroOp(), type(type) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        FuncType type;
    };

    class PopFuncTypeMicroOp : public IMicroOp
    {
    public:
        PopFuncTypeMicroOp(void) : IMicroOp() { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;
    };

    template<typename T>
    class GenericPopMicroOp : public IMicroOp
    {
    public:
        GenericPopMicroOp(T & data, std::string const & name) : IMicroOp(), data(data), name(name) { }

        virtual void handleMicroOp(MachineState & state) override
        {
            (void) state;
            data.pop();
        }

        virtual std::string toString(MachineState const & state) const override
        {
            (void) state;
            return lc3::utils::ssprintf("%s <= %s.removeTop()", name.c_str(), name.c_str());
        }

    private:
        T & data;
        std::string name;
    };

    class PrintMessageMicroOp : public IMicroOp
    {
    public:
        PrintMessageMicroOp(std::string const & msg) : IMicroOp(), msg(msg) { }

        virtual void handleMicroOp(MachineState & state) override;
        virtual std::string toString(MachineState const & state) const override;

    private:
        std::string msg;
    };

    bool isAccessViolation(uint16_t addr, MachineState const & state);
    std::pair<PIMicroOp, PIMicroOp> buildSystemModeEnter(uint16_t table_start, uint8_t vec, uint8_t priority);
};
};

#endif
