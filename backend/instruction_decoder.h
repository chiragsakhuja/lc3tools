/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef INSTRUCTION_DECODER_H
#define INSTRUCTION_DECODER_H

#include <cstdint>
#include <map>

#include "instructions.h"
#include "utils.h"

namespace lc3
{
namespace core_old
{
namespace sim
{
    class InstructionDecoder : public InstructionHandler
    {
    public:
        InstructionDecoder(void);

        optional<PIInstruction> findInstructionByEncoding(uint32_t encoding) const;

    private:
        std::map<uint32_t, std::vector<PIInstruction>> instructions_by_opcode;
    };
};
};
};

#endif
