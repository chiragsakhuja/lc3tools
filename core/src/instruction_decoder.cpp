#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <stdexcept>
#include <vector>

#include <iostream>

#include "utils.h"

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "state.h"

#include "instructions.h"
#include "instruction_decoder.h"

using namespace core;

InstructionDecoder::InstructionDecoder(void) : InstructionHandler()
{
    for(IInstruction const * inst : instructions) {
        // assumption: every instruction will have a FixedOperand first, and that is the opcode
        uint32_t opcode = ((FixedOperand *) inst->operands[0])->value;
        instructions_by_opcode[opcode].push_back(inst);
    }
}

bool InstructionDecoder::findInstructionByEncoding(uint32_t encoding, IInstruction *& candidate) const
{
    auto search = instructions_by_opcode.find(getBits(encoding, 15, 12));
    candidate = nullptr;

    if(search != instructions_by_opcode.end()) {
        for(IInstruction const * inst : search->second) {
            uint32_t cur_pos = 15;
            bool valid = true;
            for(IOperand const * op : inst->operands) {
                if(op->type == OPER_TYPE_FIXED) {
                    //printf("checking [%d, %d] => %s ?= %s\n", cur_pos, cur_pos - op->width + 1, udecToBin(getBits(encoding, cur_pos, cur_pos - op->width + 1), op->width).c_str(), udecToBin(((FixedOperand *) op)->value, op->width).c_str());
                    if(getBits(encoding, cur_pos, cur_pos - op->width + 1) != ((FixedOperand *) op)->value) {
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
