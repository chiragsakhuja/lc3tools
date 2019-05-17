#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include <memory>

#include "instructions.h"
#include "logger.h"

namespace lc3
{
namespace core
{
namespace asmbl
{
    class InstructionEncoder : public InstructionHandler
    {
    public:
        InstructionEncoder(void);

        uint32_t getDistanceToNearestInstructionName(std::string const & search) const;
        bool isValidReg(std::string const & search) const;
        std::vector<std::pair<PIInstruction, uint32_t>> getInstructionCandidates(Statement const & state) const;
        uint32_t encodeInstruction(Statement const & state, PIInstruction pattern, SymbolTable const & symbols,
            lc3::utils::AssemblerLogger & logger, bool & success) const;

    private:
        std::map<std::string, std::vector<PIInstruction>> instructions_by_name;

        uint32_t levDistance(std::string const & a, std::string const & b) const;
        uint32_t levDistanceHelper(std::string const & a, uint32_t a_len, std::string const & b, uint32_t b_len) const;
        OperType tokenTypeToOperType(TokenType type) const;
    };
};
};
};

#endif
