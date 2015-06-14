#ifndef INSTRUCTION_H
#define INSTRUCTION_H

class Instruction
{
public:
    std::string label;
    int argCount;
    int opcode;

    Instruction() {}
    Instruction(const std::string& label, int argCount, int opcode);
};

#endif
