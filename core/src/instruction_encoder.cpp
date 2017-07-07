#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "statement.h"

#include "state.h"

#include "instructions.h"
#include "instruction_encoder.h"

using namespace core;

InstructionEncoder::InstructionEncoder(void) : InstructionHandler()
{
    for(IInstruction const * inst : instructions) {
        instructions_by_name[inst->name].push_back(inst);
    }
}

bool InstructionEncoder::findInstructionByName(std::string const & search) const
{
    return instructions_by_name.find(search) != instructions_by_name.end();
}
 
bool InstructionEncoder::findInstruction(Token const * search, std::vector<IInstruction const *> & candidates) const
{
    auto inst_list = instructions_by_name.find(search->str);

    candidates.clear();
    if(inst_list != instructions_by_name.end()) {
        std::vector<IInstruction const *> const & candidate_list = inst_list->second;
        bool found_match = false;

        // check all encodings to see if there is a match
        for(IInstruction const * cur_candidate : candidate_list) {
            // first make sure the number of operands is the same, otherwise it's a waste
            if(cur_candidate->getNumOperands() == (uint32_t) search->num_opers) {
                bool actual_match = true;
                Token const * cur_oper = search->opers;

                // iterate through the oeprand types to see if the assembly matches
                for(IOperand const * oper : cur_candidate->operands) {
                    // if the operand is fixed, it won't show up in the assembly so skip it
                    if(oper->type == OPER_TYPE_FIXED) {
                        continue;
                    }

                    if(! oper->isEqualType((OperType) cur_oper->type)) {
                        actual_match = false;
                        break;
                    }

                    cur_oper = cur_oper->next;
                }

                // found a match, stop searching
                if(actual_match) {
                    candidates.clear();
                    candidates.push_back(cur_candidate);
                    found_match = true;
                    break;
                } else {
                    candidates.push_back(cur_candidate);
                }
            } else {
                candidates.push_back(cur_candidate);
            }
        }

        return found_match;
    } else {
        return false;
    }
}

bool InstructionEncoder::findReg(std::string const & search) const
{
    return regs.find(search) != regs.end();
}

// precondition: the instruction is of type pattern and is valid (no error checking)
uint32_t InstructionEncoder::encodeInstruction(IInstruction const * pattern, Token const * inst,
    std::map<std::string, uint32_t> const & symbols, AssemblerLogger & logger) const
{
    uint32_t oper_count = 1;
    uint32_t encoding = 0;

    Token const * cur_oper = inst->opers;
    for(IOperand * pattern_op : pattern->operands) {
        encoding <<= pattern_op->width;
        encoding |= pattern_op->encode(cur_oper, oper_count, regs, symbols, logger);

        if(pattern_op->type != OPER_TYPE_FIXED) {
            cur_oper = cur_oper->next;
            oper_count += 1;
        }
    }

    return encoding;
}
