#include "decoder.h"

using namespace lc3::core::sim;

Decoder::Decoder(void) : ISAHandler()
{
    for(PIInstruction inst : instructions) {
        uint16_t opcode = inst->getOperand(0)->getValue();
        instructions_by_opcode[opcode].push_back(inst);
    }
}

lc3::optional<lc3::core::PIInstruction> Decoder::decode(uint16_t value) const
{
    auto search = instructions_by_opcode.find(lc3::utils::getBits(value, 15, 12));
    bool valid = false;

    if(search != instructions_by_opcode.end()) {
        // Search instructions with the same opcode for a match.
        for(PIInstruction inst : search->second) {
            uint32_t bit_check_pos = 15;
            valid = true;
            // Scan over all fixed operands in instruction to determine if there's a match.
            for(PIOperand const op : inst->getOperands()) {
                if(op->getType() == IOperand::Type::FIXED) {
                    if(lc3::utils::getBits(value, bit_check_pos, bit_check_pos - op->getWidth() + 1) !=
                        op->getValue())
                    {
                        valid = false;
                        break;
                    }
                }
                bit_check_pos -= op->getWidth();
            }

            if(valid) {
                uint32_t bit_pos = 15;
                for(PIOperand op : inst->getOperands()) {
                    if(op->getType() != IOperand::Type::FIXED) {
                        op->setValue(lc3::utils::getBits(value, bit_pos, bit_pos - op->getWidth() + 1));
                    }
                    bit_pos -= op->getWidth();
                }
                return inst;
            }
        }
    }

    return {};
}
