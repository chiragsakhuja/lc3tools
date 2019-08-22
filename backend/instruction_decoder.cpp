/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cstdint>

#include "instruction_decoder.h"

lc3::core::sim::InstructionDecoder::InstructionDecoder(void) : InstructionHandler()
{
    for(PIInstruction inst : instructions) {
        // assumption: every instruction will have a FixedOperand first, and that is the opcode
        uint32_t opcode = std::static_pointer_cast<FixedOperand>(inst->operands[0])->value;
        instructions_by_opcode[opcode].push_back(inst);
    }
}

lc3::optional<lc3::core::PIInstruction> lc3::core::sim::InstructionDecoder::findInstructionByEncoding(uint32_t encoding)
    const
{
    auto search = instructions_by_opcode.find(utils::getBits(encoding, 15, 12));
    bool valid = false;

    if(search != instructions_by_opcode.end()) {
        for(PIInstruction inst : search->second) {
            uint32_t cur_pos = 15;
            valid = true;
            for(PIOperand const op : inst->operands) {
                if(op->type == OperType::FIXED) {
                    if(utils::getBits(encoding, cur_pos, cur_pos - op->width + 1) !=
                        std::static_pointer_cast<FixedOperand>(op)->value)
                    {
                        valid = false;
                        break;
                    }
                }
                cur_pos -= op->width;
            }

            if(valid) {
                // make a copy of our instruction so we can change the operands
                return inst;
            }
        }

        // if we reach here, we never returned true, so there was no valid match
        return {};
    }

    return {};
}
