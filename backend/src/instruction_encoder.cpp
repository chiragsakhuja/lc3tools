#include <algorithm>

#include "instruction_encoder.h"

using namespace lc3::core::asmbl;

InstructionEncoder::InstructionEncoder(void) : InstructionHandler()
{
    for(PIInstruction inst : instructions) {
        instructions_by_name[inst->name].push_back(inst);
    }
}

bool InstructionEncoder::isValidReg(std::string const & search) const
{
    return regs.find(search) != regs.end();
}

uint32_t InstructionEncoder::getDistanceToNearestInstructionName(std::string const & search) const
{
    std::string lower_search = search;
    std::transform(lower_search.begin(), lower_search.end(), lower_search.begin(), ::tolower);
    uint32_t min_distance = 0;
    bool min_set = false;
    for(auto const & inst : instructions_by_name) {
        uint32_t distance = levDistance(inst.first, lower_search);
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

std::vector<std::pair<lc3::core::PIInstruction, uint32_t>> InstructionEncoder::getInstructionCandidates(
    Statement const & state) const
{
    std::vector<std::pair<PIInstruction, uint32_t>> ret;
    StatementToken const & search = state.inst_or_pseudo;

    if(search.type == TokenType::INST) {
        for(auto const & inst : instructions_by_name) {
            uint32_t inst_dist = levDistance(inst.first, search.str);
            if(inst_dist <= search.lev_dist) {
                for(PIInstruction inst_pattern : inst.second) {
                    std::string op_string, search_string;
                    for(PIOperand op : inst_pattern->operands) {
                        if(op->type != OperType::FIXED) {
                            op_string += '0' + static_cast<char>(op->type);
                        }
                    }
                    for(StatementToken const & op : state.operands) {
                        search_string += '0' + static_cast<char>(tokenTypeToOperType(op.type));
                    }

                    uint32_t op_dist = levDistance(op_string, search_string);
                    if(op_dist < 3) {
                        if(inst_dist + op_dist == 0) {
                            ret.clear();
                            ret.push_back(std::make_pair(inst_pattern, inst_dist + op_dist));
                            break;
                        }

                        ret.push_back(std::make_pair(inst_pattern, inst_dist + op_dist));
                    }
                }
            }
        }
    }

    return ret;
}

lc3::core::OperType InstructionEncoder::tokenTypeToOperType(TokenType type) const
{
    if(type == TokenType::NUM) {
        return OperType::NUM;
    } else if(type == TokenType::REG) {
        return OperType::REG;
    } else if(type == TokenType::LABEL) {
        return OperType::LABEL;
    } else {
        return OperType::INVALID;
    }
}

uint32_t InstructionEncoder::levDistance(std::string const & a, std::string const & b) const
{
    return levDistanceHelper(a, a.size(), b, b.size());
}

uint32_t InstructionEncoder::levDistanceHelper(std::string const & a, uint32_t a_len, std::string const & b,
    uint32_t b_len) const
{
    // lazy, redundant recursive version of Levenshtein distance...may use dynamic programming eventually
    if(a_len == 0) { return b_len; }
    if(b_len == 0) { return a_len; }

    uint32_t cost = (a[a_len - 1] == b[b_len - 1]) ? 0 : 1;

    std::array<uint32_t, 3> costs;
    costs[0] = levDistanceHelper(a, a_len - 1, b, b_len    ) + 1;
    costs[1] = levDistanceHelper(a, a_len    , b, b_len - 1) + 1;
    costs[2] = levDistanceHelper(a, a_len - 1, b, b_len - 1) + cost;

    return *std::min_element(std::begin(costs), std::end(costs));
}

lc3::core::MemEntry InstructionEncoder::encodeInstruction(Statement const & state) const
{
    
}

/*
 *bool lc3::core::asmbl::InstructionEncoder::findInstructionByName(std::string const & search) const
 *{
 *    return instructions_by_name.find(search) != instructions_by_name.end();
 *}
 * 
 *bool lc3::core::asmbl::InstructionEncoder::findInstruction(OldToken const * search,
 *    std::vector<IInstruction const *> & candidates) const
 *{
 *    auto inst_list = instructions_by_name.find(search->str);
 *
 *    candidates.clear();
 *    if(inst_list != instructions_by_name.end()) {
 *        std::vector<IInstruction const *> const & candidate_list = inst_list->second;
 *        bool found_match = false;
 *
 *        // check all encodings to see if there is a match
 *        for(IInstruction const * cur_candidate : candidate_list) {
 *            // first make sure the number of operands is the same, otherwise it's a waste
 *            if(cur_candidate->getNumOperands() == (uint32_t) search->num_opers) {
 *                bool actual_match = true;
 *                OldToken const * cur_oper = search->opers;
 *
 *                // iterate through the oeprand types to see if the assembly matches
 *                for(IOperand const * oper : cur_candidate->operands) {
 *                    // if the operand is fixed, it won't show up in the assembly so skip it
 *                    if(oper->type == OperType::FIXED) {
 *                        continue;
 *                    }
 *
 *                    if(! oper->isEqualType((OperType) cur_oper->type)) {
 *                        actual_match = false;
 *                        break;
 *                    }
 *
 *                    cur_oper = cur_oper->next;
 *                }
 *
 *                // found a match, stop searching
 *                if(actual_match) {
 *                    candidates.clear();
 *                    candidates.push_back(cur_candidate);
 *                    found_match = true;
 *                    break;
 *                } else {
 *                    candidates.push_back(cur_candidate);
 *                }
 *            } else {
 *                candidates.push_back(cur_candidate);
 *            }
 *        }
 *
 *        return found_match;
 *    } else {
 *        return false;
 *    }
 *}
 *
 * precondition: the instruction is of type pattern and is valid (no error checking)
 *uint32_t lc3::core::asmbl::InstructionEncoder::encodeInstruction(IInstruction const * pattern, OldToken const * inst,
 *    std::map<std::string, uint32_t> const & symbols, lc3::utils::AssemblerLogger & logger) const
 *{
 *    uint32_t oper_count = 1;
 *    uint32_t encoding = 0;
 *
 *    OldToken const * cur_oper = inst->opers;
 *    for(IOperand * pattern_op : pattern->operands) {
 *        encoding <<= pattern_op->width;
 *        encoding |= pattern_op->encode(cur_oper, oper_count, regs, symbols, logger);
 *
 *        if(pattern_op->type != OperType::FIXED) {
 *            cur_oper = cur_oper->next;
 *            oper_count += 1;
 *        }
 *    }
 *
 *    return encoding;
 *}
 */
