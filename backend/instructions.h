/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "aliases.h"
#include "asm_types.h"
#include "logger.h"
#include "optional.h"
#include "state.h"

namespace lc3
{
namespace core
{

    enum class OperType {
          FIXED = 0
        , NUM
        , LABEL
        , REG
        , INVALID
    };

    class IOperand
    {
    public:
        OperType type;

        // used by assembler
        std::string type_str;
        uint32_t width;

        // used by simulator
        uint32_t value;

        IOperand(OperType type, std::string const & type_str, uint32_t width);
        virtual ~IOperand(void) = default;

        virtual optional<uint32_t> encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) = 0;
        bool isEqualType(OperType other) const;
    };

    class IInstruction
    {
    public:
        std::string name;
        std::vector<PIOperand> operands;

        IInstruction(std::string const & name, std::vector<PIOperand> const & operands);
        IInstruction(IInstruction const & that);
        virtual ~IInstruction(void) = default;

        uint32_t getNumOperands(void) const;
        void assignOperands(uint32_t encoded_inst);

        virtual std::vector<PIEvent> execute(MachineState const & state) = 0;
        std::string toFormatString(void) const;
        std::string toValueString(void) const;

        static std::vector<PIEvent> buildSysCallEnterHelper(MachineState const & state, uint32_t vector_id,
            MachineState::SysCallType call_type,
            std::function<uint32_t(uint32_t)> computeNewPSRValue = [](uint32_t psr_value) {
                return psr_value & 0x7fff;
            });
        static std::vector<PIEvent> buildSysCallExitHelper(MachineState const & state,
            MachineState::SysCallType call_type);
    };

    class InstructionHandler
    {
    public:
        InstructionHandler(void);
        virtual ~InstructionHandler(void) = default;

        SymbolTable const & getRegs(void) { return regs; }
    protected:
        std::vector<PIInstruction> instructions;
        SymbolTable regs;
    };

    class FixedOperand : public IOperand
    {
    public:
        FixedOperand(uint32_t width, uint32_t value) : IOperand(OperType::FIXED, "fixed", width)
            { this->value = value; }
        virtual optional<uint32_t> encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) override;
    };

    class RegOperand : public IOperand
    {
    public:
        RegOperand(uint32_t width) : IOperand(OperType::REG, "reg", width) {}
        virtual optional<uint32_t> encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) override;
    };

    class NumOperand : public IOperand
    {
    public:
        bool sext;

        NumOperand(uint32_t width, bool sext) : IOperand(OperType::NUM, "imm", width) { this->sext = sext; }
        virtual optional<uint32_t> encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) override;
    };

    class LabelOperand : public IOperand
    {
    public:
        LabelOperand(uint32_t width) : IOperand(OperType::LABEL, "label", width) {}
        virtual optional<uint32_t> encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) override;
    };

    class ADDRInstruction : public IInstruction
    {
    public:
        ADDRInstruction(void) : IInstruction("add", {
            std::make_shared<FixedOperand>(4, 0x1),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(3, 0x0),
            std::make_shared<RegOperand>(3)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class ADDIInstruction : public IInstruction
    {
    public:
        ADDIInstruction(void) : IInstruction("add", {
            std::make_shared<FixedOperand>(4, 0x1),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(1, 0x1),
            std::make_shared<NumOperand>(5, true)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class ANDRInstruction : public IInstruction
    {
    public:
        ANDRInstruction(void) : IInstruction("and", {
            std::make_shared<FixedOperand>(4, 0x5),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(3, 0x0),
            std::make_shared<RegOperand>(3)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class ANDIInstruction : public IInstruction
    {
    public:
        ANDIInstruction(void) : IInstruction("and", {
            std::make_shared<FixedOperand>(4, 0x5),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(1, 0x1),
            std::make_shared<NumOperand>(5, true)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class BRInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        BRInstruction(void) : IInstruction("br", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x7),
            std::make_shared<LabelOperand>(9)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class BRnInstruction : public BRInstruction
    {
    public:
        BRnInstruction(void) : BRInstruction("brn", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x4),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class BRzInstruction : public BRInstruction
    {
    public:
        BRzInstruction(void) : BRInstruction("brz", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x2),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class BRpInstruction : public BRInstruction
    {
    public:
        BRpInstruction(void) : BRInstruction("brp", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x1),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class BRnzInstruction : public BRInstruction
    {
    public:
        BRnzInstruction(void) : BRInstruction("brnz", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x6),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class BRzpInstruction : public BRInstruction
    {
    public:
        BRzpInstruction(void) : BRInstruction("brzp", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x3),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class BRnpInstruction : public BRInstruction
    {
    public:
        BRnpInstruction(void) : BRInstruction("brnp", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x5),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class BRnzpInstruction : public BRInstruction
    {
    public:
        BRnzpInstruction(void) : BRInstruction("brnzp", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x7),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class NOP0Instruction : public BRInstruction
    {
    public:
        NOP0Instruction(void) : BRInstruction("nop", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x0),
            std::make_shared<FixedOperand>(9, 0x0)
        }) {}
    };

    class NOP1Instruction : public BRInstruction
    {
    public:
        NOP1Instruction(void) : BRInstruction("nop", {
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(3, 0x0),
            std::make_shared<LabelOperand>(9)
        }) {}
    };

    class JMPInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        JMPInstruction(void) : IInstruction("jmp", {
            std::make_shared<FixedOperand>(4, 0xc),
            std::make_shared<FixedOperand>(3, 0x0),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(6, 0x0)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class JSRIInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        JSRIInstruction(void) : IInstruction("jsr", {
            std::make_shared<FixedOperand>(4, 0x4),
            std::make_shared<FixedOperand>(1, 0x1),
            std::make_shared<NumOperand>(11, true)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class JSRLInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        JSRLInstruction(void) : IInstruction("jsr", {
            std::make_shared<FixedOperand>(4, 0x4),
            std::make_shared<FixedOperand>(1, 0x1),
            std::make_shared<LabelOperand>(11)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class JSRRInstruction : public IInstruction
    {
    public:
        JSRRInstruction(void) : IInstruction("jsrr", {
            std::make_shared<FixedOperand>(4, 0x4),
            std::make_shared<FixedOperand>(1, 0x0),
            std::make_shared<FixedOperand>(2, 0x0),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(6, 0x0)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class LDInstruction : public IInstruction
    {
    public:
        LDInstruction(void) : IInstruction("ld", {
            std::make_shared<FixedOperand>(4, 0x2),
            std::make_shared<RegOperand>(3),
            std::make_shared<LabelOperand>(9)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class LDIInstruction : public IInstruction
    {
    public:
        LDIInstruction(void) : IInstruction("ldi", {
            std::make_shared<FixedOperand>(4, 0xa),
            std::make_shared<RegOperand>(3),
            std::make_shared<LabelOperand>(9)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class LDRInstruction : public IInstruction
    {
    public:
        LDRInstruction(void) : IInstruction("ldr", {
            std::make_shared<FixedOperand>(4, 0x6),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<NumOperand>(6, true)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class LEAIInstruction : public IInstruction
    {
    public:
        LEAIInstruction(void) : IInstruction("lea", {
            std::make_shared<FixedOperand>(4, 0xe),
            std::make_shared<RegOperand>(3),
            std::make_shared<NumOperand>(9, true)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class LEALInstruction : public IInstruction
    {
    public:
        LEALInstruction(void) : IInstruction("lea", {
            std::make_shared<FixedOperand>(4, 0xe),
            std::make_shared<RegOperand>(3),
            std::make_shared<LabelOperand>(9)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class NOTInstruction : public IInstruction
    {
    public:
        NOTInstruction(void) : IInstruction("not", {
            std::make_shared<FixedOperand>(4, 0x9),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<FixedOperand>(6, 0x3f)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class RETInstruction : public JMPInstruction
    {
    public:
        RETInstruction(void) : JMPInstruction("ret", {
            std::make_shared<FixedOperand>(4, 0xc),
            std::make_shared<FixedOperand>(3, 0x0),
            std::make_shared<FixedOperand>(3, 0x7),
            std::make_shared<FixedOperand>(6, 0x0)
        }) {}
    };

    class RTIInstruction : public IInstruction
    {
    public:
        RTIInstruction(void) : IInstruction("rti", {
            std::make_shared<FixedOperand>(4, 0x8),
            std::make_shared<FixedOperand>(12, 0x0)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class STInstruction : public IInstruction
    {
    public:
        STInstruction(void) : IInstruction("st", {
            std::make_shared<FixedOperand>(4, 0x3),
            std::make_shared<RegOperand>(3),
            std::make_shared<LabelOperand>(9)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class STIInstruction : public IInstruction
    {
    public:
        STIInstruction(void) : IInstruction("sti", {
            std::make_shared<FixedOperand>(4, 0xb),
            std::make_shared<RegOperand>(3),
            std::make_shared<LabelOperand>(9)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class STRInstruction : public IInstruction
    {
    public:
        STRInstruction(void) : IInstruction("str", {
            std::make_shared<FixedOperand>(4, 0x7),
            std::make_shared<RegOperand>(3),
            std::make_shared<RegOperand>(3),
            std::make_shared<NumOperand>(6, true)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class TRAPInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        TRAPInstruction(void) : IInstruction("trap", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<NumOperand>(8, false)
        }) {}
        virtual std::vector<PIEvent> execute(MachineState const & state) override;
    };

    class GETCInstruction : public TRAPInstruction
    {
    public:
        GETCInstruction(void) : TRAPInstruction("getc", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x20)
        }) {}
    };

    class OUTInstruction : public TRAPInstruction
    {
    public:
        OUTInstruction(void) : TRAPInstruction("out", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x21)
        }) {}
    };

    class PUTCInstruction : public TRAPInstruction
    {
    public:
        PUTCInstruction(void) : TRAPInstruction("putc", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x21)
        }) {}
    };

    class PUTSInstruction : public TRAPInstruction
    {
    public:
        PUTSInstruction(void) : TRAPInstruction("puts", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x22)
        }) {}
    };

    class INInstruction : public TRAPInstruction
    {
    public:
        INInstruction(void) : TRAPInstruction("in", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x23)
        }) {}
    };

    class PUTSPInstruction : public TRAPInstruction
    {
    public:
        PUTSPInstruction(void) : TRAPInstruction("putsp", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x24)
        }) {}
    };

    class HALTInstruction : public TRAPInstruction
    {
    public:
        HALTInstruction(void) : TRAPInstruction("halt", {
            std::make_shared<FixedOperand>(4, 0xf),
            std::make_shared<FixedOperand>(4, 0x0),
            std::make_shared<FixedOperand>(8, 0x25)
        }) {}
    };
};
};

#endif
