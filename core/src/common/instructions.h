#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "../asm/logger.h"

namespace core {
    typedef enum {
          OPER_TYPE_FIXED = 2
        , OPER_TYPE_NUM
        , OPER_TYPE_LABEL
        , OPER_TYPE_REG
    } OperType;

    std::string udecToBin(uint32_t value, uint32_t num_bits);
    std::string decToBin(int32_t value, uint32_t num_bits);
    uint32_t sextTo32(uint32_t value, uint32_t num_bits);

    class Operand
    {
    public:
        OperType type;
        std::string type_str;
        uint32_t width;

        Operand(OperType type, std::string const & type_str, uint32_t width);
        virtual ~Operand(void) = default;

        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) = 0;

        bool isEqualType(OperType other) const;
    };

    class Instruction
    {
    public:
        bool setcc;
        std::string name;
        std::vector<Operand *> operands;

        Instruction(bool setcc, std::string const & name, std::vector<Operand *> const & operands);
        ~Instruction(void);

        uint32_t getNumOperands(void);
    };

    class InstructionGenerator
    {
    public:
        std::map<std::string, std::vector<Instruction *>> instructions;
        std::map<std::string, uint32_t> regs;

        InstructionGenerator(void);
        ~InstructionGenerator(void);
    };

    class FixedOperand : public Operand
    {
    private:
        uint32_t value;
    public:
        FixedOperand(uint32_t width, uint32_t value) : Operand(OPER_TYPE_FIXED, "fixed", width) { this->value = value; }
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class RegOperand : public Operand
    {
    public:
        RegOperand(uint32_t width) : Operand(OPER_TYPE_REG, "reg", width) {}
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class NumOperand : public Operand
    {
    private:
        bool sext;
    public:
        NumOperand(uint32_t width, bool sext) : Operand(OPER_TYPE_NUM, "imm", width) { this->sext = sext; }
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class LabelOperand : public Operand
    {
    public:
        LabelOperand(uint32_t width) : Operand(OPER_TYPE_LABEL, "label", width) {}
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class ADDInstruction : public Instruction
    {
    public:
        ADDInstruction(std::vector<Operand *> const & operands) : Instruction(true, "add", operands) {}
    };

    class ANDInstruction : public Instruction
    {
    public:
        ANDInstruction(std::vector<Operand *> const & operands) : Instruction(true, "and", operands) {}
    };

    class JMPInstruction : public Instruction
    {
    public:
        JMPInstruction(std::vector<Operand *> const & operands) : Instruction(false, "jmp", operands) {}
    };

    class JSRInstruction : public Instruction
    {
    public:
        JSRInstruction(std::vector<Operand *> const & operands) : Instruction(false, "jsr", operands) {}
    };

    class LDInstruction : public Instruction
    {
    public:
        LDInstruction(std::vector<Operand *> const & operands) : Instruction(true, "ld", operands) {}
    };

    class LDIInstruction : public Instruction
    {
    public:
        LDIInstruction(std::vector<Operand *> const & operands) : Instruction(true, "ldi", operands) {}
    };

    class LDRInstruction : public Instruction
    {
    public:
        LDRInstruction(std::vector<Operand *> const & operands) : Instruction(true, "ldr", operands) {}
    };

    class LEAInstruction : public Instruction
    {
    public:
        LEAInstruction(std::vector<Operand *> const & operands) : Instruction(false, "lea", operands) {}
    };

    class NOTInstruction : public Instruction
    {
    public:
        NOTInstruction(std::vector<Operand *> const & operands) : Instruction(true, "not", operands) {}
    };

    class RTIInstruction : public Instruction
    {
    public:
        RTIInstruction(std::vector<Operand *> const & operands) : Instruction(false, "rti", operands) {}
    };

    class STInstruction : public Instruction
    {
    public:
        STInstruction(std::vector<Operand *> const & operands) : Instruction(false, "st", operands) {}
    };

    class STIInstruction : public Instruction
    {
    public:
        STIInstruction(std::vector<Operand *> const & operands) : Instruction(false, "sti", operands) {}
    };

    class STRInstruction : public Instruction
    {
    public:
        STRInstruction(std::vector<Operand *> const & operands) : Instruction(false, "str", operands) {}
    };

    class TRAPInstruction : public Instruction
    {
    public:
        TRAPInstruction(std::vector<Operand *> const & operands) : Instruction(false, "trap", operands) {}
    };

};

#endif
