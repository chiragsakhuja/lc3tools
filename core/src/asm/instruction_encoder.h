#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "logger.h"
#include "../common/printer.h"

typedef enum {
      OPER_TYPE_REG = 2
    , OPER_TYPE_IMM
    , OPER_TYPE_PCOFFS
    , OPER_TYPE_PCOFFU
    , OPER_TYPE_LABEL
    , OPER_TYPE_UNTYPED_NUM
    , OPER_TYPE_UNKNOWN
} OperType;

class Operand
{
public:
    int type;
    int hi, lo;

    Operand();
    Operand(int type, int hi, int lo);

    bool compareTypes(int other_type) const;
};

class Instruction
{
public:
    bool setcc;
    std::string label;
    std::vector<Operand *> oper_types;
    int * bit_types;

    Instruction(int width, bool setcc, std::string const & label);
    ~Instruction();
};

class InstructionEncoder
{
public:
    std::map<std::string, std::list<Instruction *> > insts;
    std::vector<std::string> regs;

    InstructionEncoder(bool print_enable, utils::Printer & printer);
    ~InstructionEncoder();

    uint32_t encodeInstruction(bool print_enable, AssemblerLogger const & logger, Instruction const * pattern, Token const * inst, uint32_t & encoded_instruction, std::string const & line) const;
    bool findReg(std::string const & search) const;

private:
    int reg_width;
    std::string udec_to_bin(uint32_t x, uint32_t num_bits) const;
};

#endif
