#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "state.h"

#include "instructions.h"
#include "instruction_encoder.h"

using namespace core;

InstructionEncoder::InstructionEncoder(void) : InstructionHandler()
{
    for(Instruction const * inst : instructions) {
        instructions_by_name[inst->name].push_back(inst);
    }
}

bool InstructionEncoder::findInstructionByName(std::string const & search) const
{
    return instructions_by_name.find(search) != instructions_by_name.end();
}

bool InstructionEncoder::findInstruction(Token const * search, std::vector<Instruction const *> & candidates) const
{
    auto inst_list = instructions_by_name.find(search->str);
    candidates.clear();
    if(inst_list != instructions_by_name.end()) {
        std::vector<Instruction const *> const & candidate_list = inst_list->second;
        bool found_match = false;

        // check all encodings to see if there is a match
        for(Instruction const * cur_candidate : candidate_list) {
            // first make sure the number of operands is the same, otherwise it's a waste
            if(cur_candidate->getNumOperands() == (uint32_t) search->num_operands) {
                bool actual_match = true;
                Token const * cur_oper = search->opers;

                // iterate through the oeprand types to see if the assembly matches
                for(Operand const * oper : cur_candidate->operands) {
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
void InstructionEncoder::encodeInstruction(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Instruction const * pattern,
    Token const * inst, uint32_t & encoded_instruction,
    std::map<std::string, uint32_t> const & labels) const
{
    uint32_t oper_count = 1;
    encoded_instruction = 0;

    Token const * cur_oper = inst->opers;
    for(Operand * pattern_op : pattern->operands) {
        encoded_instruction <<= pattern_op->width;
        encoded_instruction |= pattern_op->encode(log_enable, logger, filename, line, inst, cur_oper, oper_count, regs, labels);

        if(pattern_op->type != OPER_TYPE_FIXED) {
            cur_oper = cur_oper->next;
            oper_count += 1;
        }
    }
}
