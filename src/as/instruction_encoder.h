#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include <map>
#include <vector>
#include <list>
#include <string>

#include "../utils/printer.h"
#include "logger.h"

typedef enum {
      OPER_TYPE_REG = 2
    , OPER_TYPE_IMM
    , OPER_TYPE_PCOFFS
    , OPER_TYPE_PCOFFU
    , OPER_TYPE_LABEL
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

    InstructionEncoder(bool print_enable, utils::Printer const & printer);
    ~InstructionEncoder();

    bool encodeInstruction(bool print_enable, AssemblerLogger const & logger, Instruction const * pattern, Token const * inst, uint32_t & encoded_instruction) const;

private:
    int reg_width;
};

#endif
