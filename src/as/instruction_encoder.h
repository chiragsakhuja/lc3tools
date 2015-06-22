#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

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
    std::vector<Operand> argTypes;
    int *bitTypes;

    Instruction(int width, bool setcc, const std::string& label);
    ~Instruction();
};

class InstructionEncoder
{
public:
    static InstructionEncoder& getInstance();

    static std::map<int, std::list<Instruction *> > insts;

private:
    static int regWidth;
    static std::vector<std::string> regs;

    InstructionEncoder();
};

#endif
