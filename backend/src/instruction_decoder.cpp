#include <cstdint>

#include "instruction_decoder.h"

lc3::core::InstructionDecoder::InstructionDecoder(void) : InstructionHandler()
{
    for(IInstruction const * inst : instructions) {
        // assumption: every instruction will have a FixedOperand first, and that is the opcode
        uint32_t opcode = ((FixedOperand *) inst->operands[0])->value;
        instructions_by_opcode[opcode].push_back(inst);
    }
}

bool lc3::core::InstructionDecoder::findInstructionByEncoding(uint32_t encoding,
    lc3::core::IInstruction *& candidate) const
{
    auto search = instructions_by_opcode.find(utils::getBits(encoding, 15, 12));
    candidate = nullptr;

    if(search != instructions_by_opcode.end()) {
        for(IInstruction const * inst : search->second) {
            uint32_t cur_pos = 15;
            bool valid = true;
            for(IOperand const * op : inst->operands) {
                if(op->type == OperType::OPER_TYPE_FIXED) {
                    if(utils::getBits(encoding, cur_pos, cur_pos - op->width + 1) != ((FixedOperand *) op)->value) {
                        valid = false;
                        break;
                    }
                }
                cur_pos -= op->width;
            }

            if(valid) {
                // make a copy of our instruction so we can change the operands
                candidate = inst->clone();
                return true;
            }
        }

        // if we reach here, we never returned true, so there was no valid match
        return false;
    } else {
        return false;
    }
}
