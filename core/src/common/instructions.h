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

    class Operand
    {
    public:
        OperType type;
        uint32_t width;

        Operand(OperType type, uint32_t width);
        virtual ~Operand(void) = default;

        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, Token const * inst, Token const * operand, uint32_t oper_count, std::map<std::string, uint32_t> const & registers) = 0;
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
        FixedOperand(uint32_t width, uint32_t value) : Operand(OPER_TYPE_FIXED, width) { this->value = value; }
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, Token const * inst, Token const * operand, uint32_t oper_count, std::map<std::string, uint32_t> const & registers) override;
    };

    class RegOperand : public Operand
    {
    public:
        RegOperand(uint32_t width) : Operand(OPER_TYPE_REG, width) {}
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, Token const * inst, Token const * operand, uint32_t oper_count, std::map<std::string, uint32_t> const & registers) override;
    };

    class NumOperand : public Operand
    {
    private:
        bool sext;
    public:
        NumOperand(uint32_t width, bool sext) : Operand(OPER_TYPE_NUM, width) { this->sext = sext; }
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, Token const * inst, Token const * operand, uint32_t oper_count, std::map<std::string, uint32_t> const & registers) override;
    };

    class ADDInstruction : public Instruction
    {
    public:
        ADDInstruction(std::vector<Operand *> const & operands) : Instruction(true, "add", operands) {}
    };
};

#endif
