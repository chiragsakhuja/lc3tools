#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "logger.h"
#include "../common/printer.h"
#include "../common/json_loader.h"

class InstructionEncoder
{
public:
    InstructionEncoder(void);

    uint32_t encodeInstruction(bool print_enable, AssemblerLogger const & logger, Instruction const * pattern, Token const * inst, uint32_t & encoded_instruction, std::string const & line) const;
    bool findReg(std::string const & search) const;

private:
    JSONLoader encodings;
    std::string udec_to_bin(uint32_t x, uint32_t num_bits) const;
};

#endif
