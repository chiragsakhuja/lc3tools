#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

namespace core {
    typedef enum {
          OPER_TYPE_FIXED = 2
        , OPER_TYPE_NUM
        , OPER_TYPE_LABEL
        , OPER_TYPE_REG
    } OperType;

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

        virtual uint32_t encode(Token const * oper, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels,
            AssemblerLogger & logger) = 0;
        virtual IOperand * clone(void) const = 0;
        bool isEqualType(OperType other) const;
    };

    class IInstruction
    {
    public:
        std::string name;
        std::vector<IOperand *> operands;

        IInstruction(std::string const & name, std::vector<IOperand *> const & operands);
        IInstruction(IInstruction const & that);
        virtual ~IInstruction(void);

        uint32_t getNumOperands(void) const;
        void assignOperands(uint32_t encoded_inst);

        virtual std::vector<IStateChange const *> execute(MachineState const & state) = 0;
        virtual IInstruction * clone(void) const = 0;
        std::string toFormatString(void) const;
        std::string toValueString(void) const;
    };

    class InstructionHandler
    {
    public:
        InstructionHandler(void);
        virtual ~InstructionHandler(void);

        std::map<std::string, uint32_t> const & getRegs(void) { return regs; }
    protected:
        std::vector<IInstruction *> instructions;
        std::map<std::string, uint32_t> regs;
    };

    class FixedOperand : public IOperand
    {
    public:
        FixedOperand(uint32_t width, uint32_t value) : IOperand(OPER_TYPE_FIXED, "fixed", width)
            { this->value = value; }
        virtual uint32_t encode(Token const * oper, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels,
            AssemblerLogger & logger) override;
        virtual FixedOperand * clone(void) const override { return new FixedOperand(*this); }
    };

    class RegOperand : public IOperand
    {
    public:
        RegOperand(uint32_t width) : IOperand(OPER_TYPE_REG, "reg", width) {}
        virtual uint32_t encode(Token const * oper, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels,
            AssemblerLogger & logger) override;
        virtual RegOperand * clone(void) const override { return new RegOperand(*this); }
    };

    class NumOperand : public IOperand
    {
    public:
        bool sext;

        NumOperand(uint32_t width, bool sext) : IOperand(OPER_TYPE_NUM, "imm", width) { this->sext = sext; }
        virtual uint32_t encode(Token const * oper, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels,
            AssemblerLogger & logger) override;
        virtual NumOperand * clone(void) const override { return new NumOperand(*this); }
    };

    class LabelOperand : public IOperand
    {
    public:
        LabelOperand(uint32_t width) : IOperand(OPER_TYPE_LABEL, "label", width) {}
        virtual uint32_t encode(Token const * oper, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels,
            AssemblerLogger & logger) override;
        virtual LabelOperand * clone(void) const override { return new LabelOperand(*this); }
    };

    class ADDRInstruction : public IInstruction
    {
    public:
        ADDRInstruction(void) : IInstruction("add", {
            new FixedOperand(4, 0x1),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(3, 0x0),
            new RegOperand(3)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual ADDRInstruction * clone(void) const override { return new ADDRInstruction(*this); }
    };

    class ADDIInstruction : public IInstruction
    {
    public:
        ADDIInstruction(void) : IInstruction("add", {
            new FixedOperand(4, 0x1),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(1, 0x1),
            new NumOperand(5, true)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual ADDIInstruction * clone(void) const override { return new ADDIInstruction(*this); }
    };

    class ANDRInstruction : public IInstruction
    {
    public:
        ANDRInstruction(void) : IInstruction("and", {
            new FixedOperand(4, 0x5),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(3, 0x0),
            new RegOperand(3)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual ANDRInstruction * clone(void) const override { return new ANDRInstruction(*this); }
    };

    class ANDIInstruction : public IInstruction
    {
    public:
        ANDIInstruction(void) : IInstruction("and", {
            new FixedOperand(4, 0x5),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(1, 0x1),
            new NumOperand(5, true)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual ANDIInstruction * clone(void) const override { return new ANDIInstruction(*this); }
    };

    class BRInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        BRInstruction(void) : IInstruction("br", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x7),
            new LabelOperand(9)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual BRInstruction * clone(void) const override { return new BRInstruction(*this); }
    };

    class BRnInstruction : public BRInstruction
    {
    public:
        BRnInstruction(void) : BRInstruction("brn", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x4),
            new LabelOperand(9)
        }) {}
        virtual BRnInstruction * clone(void) const override { return new BRnInstruction(*this); }
    };

    class BRzInstruction : public BRInstruction
    {
    public:
        BRzInstruction(void) : BRInstruction("brz", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x2),
            new LabelOperand(9)
        }) {}
        virtual BRzInstruction * clone(void) const override { return new BRzInstruction(*this); }
    };

    class BRpInstruction : public BRInstruction
    {
    public:
        BRpInstruction(void) : BRInstruction("brp", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x1),
            new LabelOperand(9)
        }) {}
        virtual BRpInstruction * clone(void) const override { return new BRpInstruction(*this); }
    };

    class BRnzInstruction : public BRInstruction
    {
    public:
        BRnzInstruction(void) : BRInstruction("brnz", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x6),
            new LabelOperand(9)
        }) {}
        virtual BRnzInstruction * clone(void) const override { return new BRnzInstruction(*this); }
    };

    class BRzpInstruction : public BRInstruction
    {
    public:
        BRzpInstruction(void) : BRInstruction("brzp", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x3),
            new LabelOperand(9)
        }) {}
        virtual BRzpInstruction * clone(void) const override { return new BRzpInstruction(*this); }
    };

    class BRnpInstruction : public BRInstruction
    {
    public:
        BRnpInstruction(void) : BRInstruction("brnp", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x5),
            new LabelOperand(9)
        }) {}
        virtual BRnpInstruction * clone(void) const override { return new BRnpInstruction(*this); }
    };

    class BRnzpInstruction : public BRInstruction
    {
    public:
        BRnzpInstruction(void) : BRInstruction("brnzp", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x7),
            new LabelOperand(9)
        }) {}
        virtual BRnzpInstruction * clone(void) const override { return new BRnzpInstruction(*this); }
    };

    class NOP0Instruction : public BRInstruction
    {
    public:
        NOP0Instruction(void) : BRInstruction("nop", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x0),
            new FixedOperand(9, 0x0)
        }) {}
        virtual NOP0Instruction * clone(void) const override { return new NOP0Instruction(*this); }
    };

    class NOP1Instruction : public BRInstruction
    {
    public:
        NOP1Instruction(void) : BRInstruction("nop", {
            new FixedOperand(4, 0x0),
            new FixedOperand(3, 0x0),
            new LabelOperand(9)
        }) {}
        virtual NOP1Instruction * clone(void) const override { return new NOP1Instruction(*this); }
    };

    class JMPInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        JMPInstruction(void) : IInstruction("jmp", {
            new FixedOperand(4, 0xc),
            new FixedOperand(3, 0x0),
            new RegOperand(3),
            new FixedOperand(6, 0x0)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual JMPInstruction * clone(void) const override { return new JMPInstruction(*this); }
    };

    class JSRInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        JSRInstruction(void) : IInstruction("jsr", {
            new FixedOperand(4, 0x4),
            new FixedOperand(1, 0x1),
            new LabelOperand(11)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual JSRInstruction * clone(void) const override { return new JSRInstruction(*this); }
    };

    class JSRRInstruction : public IInstruction
    {
    public:
        JSRRInstruction(void) : IInstruction("jsrr", {
            new FixedOperand(4, 0x4),
            new FixedOperand(1, 0x0),
            new FixedOperand(2, 0x0),
            new RegOperand(3),
            new FixedOperand(6, 0x0)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual JSRRInstruction * clone(void) const override { return new JSRRInstruction(*this); }
    };

    class LDInstruction : public IInstruction
    {
    public:
        LDInstruction(void) : IInstruction("ld", {
            new FixedOperand(4, 0x2),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual LDInstruction * clone(void) const override { return new LDInstruction(*this); }
    };

    class LDIInstruction : public IInstruction
    {
    public:
        LDIInstruction(void) : IInstruction("ldi", {
            new FixedOperand(4, 0xa),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual LDIInstruction * clone(void) const override { return new LDIInstruction(*this); }
    };

    class LDRInstruction : public IInstruction
    {
    public:
        LDRInstruction(void) : IInstruction("ldr", {
            new FixedOperand(4, 0x6),
            new RegOperand(3),
            new RegOperand(3),
            new NumOperand(6, true)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual LDRInstruction * clone(void) const override { return new LDRInstruction(*this); }
    };

    class LEAInstruction : public IInstruction
    {
    public:
        LEAInstruction(void) : IInstruction("lea", {
            new FixedOperand(4, 0xe),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual LEAInstruction * clone(void) const override { return new LEAInstruction(*this); }
    };

    class NOTInstruction : public IInstruction
    {
    public:
        NOTInstruction(void) : IInstruction("not", {
            new FixedOperand(4, 0x0),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(6, 0x3f)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual NOTInstruction * clone(void) const override { return new NOTInstruction(*this); }
    };

    class RETInstruction : public JMPInstruction
    {
    public:
        RETInstruction(void) : JMPInstruction("ret", {
            new FixedOperand(4, 0xc),
            new FixedOperand(3, 0x0),
            new FixedOperand(3, 0x7),
            new FixedOperand(6, 0x0)
        }) {}
        virtual RETInstruction * clone(void) const override { return new RETInstruction(*this); }
    };

    class RTIInstruction : public IInstruction
    {
    public:
        RTIInstruction(void) : IInstruction("rti", {
            new FixedOperand(4, 0x8),
            new FixedOperand(12, 0x0)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual RTIInstruction * clone(void) const override { return new RTIInstruction(*this); }
    };

    class STInstruction : public IInstruction
    {
    public:
        STInstruction(void) : IInstruction("st", {
            new FixedOperand(4, 0x3),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual STInstruction * clone(void) const override { return new STInstruction(*this); }
    };

    class STIInstruction : public IInstruction
    {
    public:
        STIInstruction(void) : IInstruction("sti", {
            new FixedOperand(4, 0xb),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual STIInstruction * clone(void) const override { return new STIInstruction(*this); }
    };

    class STRInstruction : public IInstruction
    {
    public:
        STRInstruction(void) : IInstruction("str", {
            new FixedOperand(4, 0x7),
            new RegOperand(3),
            new RegOperand(3),
            new NumOperand(6, true)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual STRInstruction * clone(void) const override { return new STRInstruction(*this); }
    };

    class TRAPInstruction : public IInstruction
    {
    public:
        using IInstruction::IInstruction;
        TRAPInstruction(void) : IInstruction("trap", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new NumOperand(8, false)
        }) {}
        virtual std::vector<IStateChange const *> execute(MachineState const & state) override;
        virtual TRAPInstruction * clone(void) const override { return new TRAPInstruction(*this); }
    };

    class GETCInstruction : public TRAPInstruction
    {
    public:
        GETCInstruction(void) : TRAPInstruction("getc", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new FixedOperand(8, 0x20)
        }) {}
        virtual GETCInstruction * clone(void) const override { return new GETCInstruction(*this); }
    };

    class OUTInstruction : public TRAPInstruction
    {
    public:
        OUTInstruction(void) : TRAPInstruction("out", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new FixedOperand(8, 0x21)
        }) {}
        virtual OUTInstruction * clone(void) const override { return new OUTInstruction(*this); }
    };

    class PUTSInstruction : public TRAPInstruction
    {
    public:
        PUTSInstruction(void) : TRAPInstruction("puts", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new FixedOperand(8, 0x22)
        }) {}
        virtual PUTSInstruction * clone(void) const override { return new PUTSInstruction(*this); }
    };

    class INInstruction : public TRAPInstruction
    {
    public:
        INInstruction(void) : TRAPInstruction("in", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new FixedOperand(8, 0x23)
        }) {}
        virtual INInstruction * clone(void) const override { return new INInstruction(*this); }
    };

    class PUTSPInstruction : public TRAPInstruction
    {
    public:
        PUTSPInstruction(void) : TRAPInstruction("putsp", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new FixedOperand(8, 0x24)
        }) {}
        virtual PUTSPInstruction * clone(void) const override { return new PUTSPInstruction(*this); }
    };

    class HALTInstruction : public TRAPInstruction
    {
    public:
        HALTInstruction(void) : TRAPInstruction("halt", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new FixedOperand(8, 0x25)
        }) {}
        virtual HALTInstruction * clone(void) const override { return new HALTInstruction(*this); }
    };
};

#endif
