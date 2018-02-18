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
    std::string lower_search = search;
    std::transform(lower_search.begin(), lower_search.end(), lower_search.begin(), ::tolower);
    return regs.find(lower_search) != regs.end();
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

uint32_t InstructionEncoder::encodeInstruction(Statement const & state, PIInstruction pattern,
    SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger, bool & success) const
{
    uint32_t encoding = 0;
    success = true;

    uint32_t oper_count = 0;
    bool first = true;

    for(PIOperand op : pattern->operands) {
        StatementToken tok;
        if(op->type == OperType::FIXED) {
            if(first) {
                first = false;
                tok = state.inst_or_pseudo;
            }
        } else {
            tok = state.operands[oper_count];
        }

        bool encode_success = true;
        encoding <<= op->width;
        encoding |= op->encode(tok, oper_count, regs, symbols, logger, encode_success);
        success &= encode_success;

        if(op->type != OperType::FIXED) {
            oper_count += 1;
        }
    }

    return encoding;
}
