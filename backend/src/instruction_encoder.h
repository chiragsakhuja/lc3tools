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

        uint32_t getDistanceToNearestInstructionName(std::string const & search) const;
        bool checkIfReg(std::string const & search) const;

        bool findInstructionByName(std::string const & search) const;
        bool findInstruction(Token const * search, std::vector<IInstruction const *> & candidates) const;

        // precondition: the instruction is of type pattern and is valid (no error checking)
        uint32_t encodeInstruction(IInstruction const * pattern, Token const * inst,
            std::map<std::string, uint32_t> const & symbols, lc3::utils::AssemblerLogger & logger) const;
    private:
        static constexpr uint32_t lev_thresh = 3;
        std::map<std::string, std::vector<IInstruction const *>> instructions_by_name;

        uint32_t levDistance(std::string const & a, uint32_t a_len, std::string const & b, uint32_t b_len) const;
    };
};

#endif
