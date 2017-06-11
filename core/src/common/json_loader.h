#ifndef JSON_LOADER_H
#define JSON_LOADER_H

#include <string>
#include <vector>

#include "printer.h"

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

    Operand(void);
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

class JSONLoader
{
public:
    std::map<std::string, std::list<Instruction *> > insts;
    std::vector<std::string> regs;

    JSONLoader(void);
    ~JSONLoader(void);

    void init(bool print_enable, utils::Printer & printer);

private:
    int reg_width;
};

#endif
