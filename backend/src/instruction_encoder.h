#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "instructions.h"
#include "logger.h"

namespace lc3::core
{
    class InstructionEncoder : public InstructionHandler
    {
    public:
        InstructionEncoder(void);

        bool findInstructionByName(std::string const & search) const;
        bool findReg(std::string const & search) const;
        bool findInstruction(Token const * search, std::vector<IInstruction const *> & candidates) const;

        // precondition: the instruction is of type pattern and is valid (no error checking)
        uint32_t encodeInstruction(IInstruction const * pattern, Token const * inst,
            std::map<std::string, uint32_t> const & symbols, lc3::utils::AssemblerLogger & logger) const;
    private:
        std::map<std::string, std::vector<IInstruction const *>> instructions_by_name;
    };
};

#endif
