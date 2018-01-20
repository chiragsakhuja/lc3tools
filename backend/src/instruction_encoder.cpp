#include <algorithm>

#include "instruction_encoder.h"

lc3::core::InstructionEncoder::InstructionEncoder(void) : InstructionHandler()
{
    for(IInstruction const * inst : instructions) {
        instructions_by_name[inst->name].push_back(inst);
    }
}

bool lc3::core::InstructionEncoder::checkIfReg(std::string const & search) const
{
    return regs.find(search) != regs.end();
}

uint32_t lc3::core::InstructionEncoder::getDistanceToNearestInstructionName(std::string const & search) const
{
    uint32_t min_distance = 0;
    bool min_set = false;
    for(auto const & inst : instructions_by_name) {
        uint32_t distance = levDistance(inst.first, inst.first.size(), search, search.size());
        if(! min_set) {
            min_distance = distance;
            min_set = true;
        }
        if(distance < min_distance) {
            min_distance = distance;
        }
    }

    return min_distance;
}

uint32_t lc3::core::InstructionEncoder::levDistance(std::string const & a, uint32_t a_len, std::string const & b,
    uint32_t b_len) const
{
    // lazy, redundant recursive version of Levenshtein distance...may use dynamic programming eventually
    if(a_len == 0) { return b_len; }
    if(b_len == 0) { return a_len; }

    uint32_t cost = (a[a_len - 1] == b[b_len - 1]) ? 0 : 1;

    std::array<uint32_t, 3> costs;
    costs[0] = levDistance(a, a_len - 1, b, b_len    ) + 1;
    costs[1] = levDistance(a, a_len    , b, b_len - 1) + 1;
    costs[2] = levDistance(a, a_len - 1, b, b_len - 1) + cost;

    return *std::min_element(std::begin(costs), std::end(costs));
}

bool lc3::core::InstructionEncoder::findInstructionByName(std::string const & search) const
{
    return instructions_by_name.find(search) != instructions_by_name.end();
}
 
bool lc3::core::InstructionEncoder::findInstruction(Token const * search, std::vector<IInstruction const *> & candidates) const
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
                    if(oper->type == OperType::OPER_TYPE_FIXED) {
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

// precondition: the instruction is of type pattern and is valid (no error checking)
uint32_t lc3::core::InstructionEncoder::encodeInstruction(IInstruction const * pattern, Token const * inst,
    std::map<std::string, uint32_t> const & symbols, lc3::utils::AssemblerLogger & logger) const
{
    uint32_t oper_count = 1;
    uint32_t encoding = 0;

    Token const * cur_oper = inst->opers;
    for(IOperand * pattern_op : pattern->operands) {
        encoding <<= pattern_op->width;
        encoding |= pattern_op->encode(cur_oper, oper_count, regs, symbols, logger);

        if(pattern_op->type != OperType::OPER_TYPE_FIXED) {
            cur_oper = cur_oper->next;
            oper_count += 1;
        }
    }

    return encoding;
}
