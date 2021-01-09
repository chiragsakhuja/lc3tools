/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <memory>
#include <string>
#include <vector>

#include "aliases.h"
#include "isa_abstract.h"

namespace lc3
{
namespace core
{
    class MachineState;

    class ADDRegInstruction : public IInstruction
    {
    public:
        ADDRegInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class ADDImmInstruction : public IInstruction
    {
    public:
        ADDImmInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class ANDRegInstruction : public IInstruction
    {
    public:
        ANDRegInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class ANDImmInstruction : public IInstruction
    {
    public:
        ANDImmInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class BRInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;

        BRInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class BRzInstruction : public BRInstruction
    {
    public:
        BRzInstruction(void);
    };

    class BRnInstruction : public BRInstruction
    {
    public:
        BRnInstruction(void);
    };

    class BRpInstruction : public BRInstruction
    {
    public:
        BRpInstruction(void);
    };

    class BRnzInstruction : public BRInstruction
    {
    public:
        BRnzInstruction(void);
    };

    class BRzpInstruction : public BRInstruction
    {
    public:
        BRzpInstruction(void);
    };

    class BRnpInstruction : public BRInstruction
    {
    public:
        BRnpInstruction(void);
    };

    class BRnzpInstruction : public BRInstruction
    {
    public:
        BRnzpInstruction(void);
    };

    class NOP0Instruction : public BRInstruction
    {
    public:
        NOP0Instruction(void);
    };

    class NOP1Instruction : public BRInstruction
    {
    public:
        NOP1Instruction(void);
    };

    class JMPInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;

        JMPInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class JSRRInstruction : public IInstruction
    {
    public:
        JSRRInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class JSRInstruction : public IInstruction
    {
    public:
        JSRInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class LDInstruction : public IInstruction
    {
    public:
        LDInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class LDIInstruction : public IInstruction
    {
    public:
        LDIInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class LDRInstruction : public IInstruction
    {
    public:
        LDRInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class LEAInstruction : public IInstruction
    {
    public:
        LEAInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class NOTInstruction : public IInstruction
    {
    public:
        NOTInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class RETInstruction : public JMPInstruction
    {
    public:
        RETInstruction(void);
    };

    class RTIInstruction : public IInstruction
    {
    public:
        RTIInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class STInstruction : public IInstruction
    {
    public:
        STInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class STIInstruction : public IInstruction
    {
    public:
        STIInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class STRInstruction : public IInstruction
    {
    public:
        STRInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class TRAPInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;

        TRAPInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class GETCInstruction : public TRAPInstruction
    {
    public:
        GETCInstruction(void);
    };

    class OUTInstruction : public TRAPInstruction
    {
    public:
        OUTInstruction(void);
    };

    class PUTCInstruction : public TRAPInstruction
    {
    public:
        PUTCInstruction(void);
    };

    class PUTSInstruction : public TRAPInstruction
    {
    public:
        PUTSInstruction(void);
    };

    class INInstruction : public TRAPInstruction
    {
    public:
        INInstruction(void);
    };

    class PUTSPInstruction : public TRAPInstruction
    {
    public:
        PUTSPInstruction(void);
    };

    class HALTInstruction : public TRAPInstruction
    {
    public:
        HALTInstruction(void);
    };
};
};

#endif
