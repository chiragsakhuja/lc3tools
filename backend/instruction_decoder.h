#ifndef INSTRUCTION_DECODER_H
#define INSTRUCTION_DECODER_H

#include <cstdint>
#include <map>

#include "instructions.h"

namespace lc3
{
namespace core
{
namespace sim
{
    class InstructionDecoder : public InstructionHandler
    {
    public:
        InstructionDecoder(void);

        PIInstruction findInstructionByEncoding(uint32_t encoding, bool & valid) const;

    private:
        std::map<uint32_t, std::vector<PIInstruction>> instructions_by_opcode;
    };
};
};
};

#endif
