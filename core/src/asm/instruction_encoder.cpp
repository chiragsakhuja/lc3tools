#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "instruction_encoder.h"
#include "logger.h"
#include "../common/printer.h"

using namespace core;

// precondition: the instruction is of type pattern and is valid (no error checking)
void InstructionEncoder::encodeInstruction(bool log_enable, AssemblerLogger const & logger,
    Instruction const * pattern, Token const * inst, uint32_t & encoded_instruction,
    std::string const & line) const
{
    uint32_t oper_count = 1;
    encoded_instruction = 0;

    Token const * cur_oper = inst->opers;
    for(Operand * pattern_op : pattern->operands) {
        uint32_t encoded_operand = 0;
        encoded_instruction <<= pattern_op->width;
        pattern_op->encode(log_enable, logger, inst, cur_oper, encoded_operand, instructions.regs);
        encoded_instruction |= encoded_operand;
        oper_count += 1;
    }
}

bool InstructionEncoder::findInstruction(Token const * search, Instruction * candidate) const
{
    auto inst_list = instructions.instructions.find(std::string(search->str));
    if(inst_list != instructions.instructions.end()) {
        std::vector<Instruction *> const & candidates = inst_list->second;
        bool found_match = false;

        // check all encodings to see if there is a match
        for(Instruction * cur_candidate : candidates) {
            // first make sure the number of operands is the same, otherwise it's a waste
            if(cur_candidate->operands.size() == (uint32_t) search->num_operands) {
                // TODO: what if there are multiple candidates?
                candidate = cur_candidate;

                bool actual_match = true;
                Token const * cur_oper = search->opers;

                // iterate through the oeprand types to see if the assembly matches
                for(Operand const * oper : cur_candidate->operands) {
                    /*
                     *if(! oper->compareTypes(cur_oper->type)) {
                     *    actual_match = false;
                     *    break;
                     *}
                     */

                    cur_oper = cur_oper->next;
                }

                // found a match, stop searching
                if(actual_match) {
                    found_match = true;
                    break;
                }
            }
        }

        return found_match;
    } else {
        return false;
    }
}

bool InstructionEncoder::findReg(std::string const & search) const
{
    // TODO: write function
    return true;
}
