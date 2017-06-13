#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "logger.h"
#include "../common/instructions.h"
#include "../common/printer.h"

namespace core
{
    class InstructionEncoder
    {
    public:
        InstructionEncoder(InstructionGenerator const & instructions) : instructions(instructions) {}

        // precondition: the instruction is of type pattern and is valid (no error checking)
        void encodeInstruction(bool log_enable, AssemblerLogger const & logger, Instruction const * pattern, Token const * inst, uint32_t & encoded_instruction, std::string const & line) const;
        bool findReg(std::string const & search) const;
        bool findInstruction(Token const * search, Instruction * candidate) const;

    private:
        InstructionGenerator const & instructions;
        std::string udec_to_bin(uint32_t x, uint32_t num_bits) const;
    };
};

#endif
