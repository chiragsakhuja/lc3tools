#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

typedef enum {
      ARG_TYPE_REG = 2
    , ARG_TYPE_IMMS
    , ARG_TYPE_IMMU
    , ARG_TYPE_PCOFFS
    , ARG_TYPE_PCOFFU
    , ARG_TYPE_UNKNOWN
} AsArgType;

class Operand
{
public:
    int type;
    int hi, lo;

    Operand();
    Operand(int type, int hi, int lo);
};

class Instruction
{
public:
    bool setcc;
    std::string label;
    std::vector<Operand *> argTypes;
    int *bitTypes;

    Instruction(int width, bool setcc, const std::string& label);
    ~Instruction();
};

class InstructionEncoder
{
public:
    static InstructionEncoder& getInstance();

    static std::map<std::string, std::list<Instruction *> > insts;
    static std::vector<std::string> regs;

private:
    static int regWidth;

    InstructionEncoder();
    ~InstructionEncoder();
};

#endif
