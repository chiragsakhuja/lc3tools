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

InstructionEncoder::InstructionEncoder(bool print_enable, utils::Printer & printer)
{
    encodings.init();
}

// precondition: the instruction is of type pattern and is valid (no error checking)
uint32_t InstructionEncoder::encodeInstruction(bool print_enable, AssemblerLogger const & logger, Instruction const * pattern, Token const * inst, uint32_t & encoded_instruction, std::string const & line) const
{
    uint32_t status = 0;

    int * bits = new int[reg_width];
    std::fill_n(bits, reg_width, OPER_TYPE_UNKNOWN);

    if(print_enable) {
        logger.printf(utils::PrintType::DEBUG, "%d  : %s", inst->row_num, line.c_str());
    }

    Token const * cur_oper = inst->opers;
    uint32_t oper_count = 1;
    for(auto it = pattern->oper_types.begin(); it != pattern->oper_types.end(); ++it) {
        int type = (*it)->type;
        uint32_t token_val;
        uint32_t num_bits = (*it)->hi - (*it)->lo + 1;

        if(type == OPER_TYPE_REG) {
            for(token_val = 0; token_val < regs.size(); token_val += 1) {
                if(regs[token_val] == cur_oper->str) {
                    break;
                }
            }

            if(print_enable) {
                logger.printf(utils::PrintType::DEBUG, "%d.%d: reg %s => %s", inst->row_num, oper_count, cur_oper->str.c_str(), udec_to_bin(token_val, num_bits).c_str());
            }
        } else if(type == OPER_TYPE_IMM) {
            token_val = cur_oper->num;

            if(print_enable) {
                logger.printf(utils::PrintType::DEBUG, "%d.%d: imm %d => %s", inst->row_num, oper_count, cur_oper->num, udec_to_bin(token_val, num_bits).c_str());
            }
        }
        // TODO: add other types here

        for(uint32_t i = (*it)->lo; i <= (*it)->hi; i += 1) {
            bits[i] = token_val & 1;
            token_val >>= 1;
        }

        cur_oper = cur_oper->next;
        oper_count += 1;
    }

    for(int i = 0; i < reg_width; i += 1) {
        if(pattern->bit_types[i] == 1 || pattern->bit_types[i] == 0) {
            bits[i] = pattern->bit_types[i];
        }
    }

    // sanity check
    std::stringstream output;
    for(int i = reg_width - 1; i >= 0; i -= 1) {
        if(bits[i] == OPER_TYPE_UNKNOWN) {
            status |= 1;
            break;
        } else {
            if(print_enable) {
                output << bits[i];
            }
        }
    }

    if(print_enable) {
        logger.printf(utils::PrintType::DEBUG, "%d  : %s => %s", inst->row_num, line.c_str(), output.str().c_str());
    }

    if(status == 0) {
        encoded_instruction = 0;

        for(int i = reg_width - 1; i >= 0; i -= 1) {
            encoded_instruction <<= 1;
            encoded_instruction |= bits[i];
        }
    }

    delete[] bits;

    return status;
}

bool InstructionEncoder::findReg(std::string const & search) const
{
    return std::find(regs.begin(), regs.end(), search) != std::end(regs);
}

std::string InstructionEncoder::udec_to_bin(uint32_t x, uint32_t num_bits) const
{
    char * bits = new char[num_bits + 1];
    std::fill_n(bits, num_bits, '0');
    bits[num_bits] = 0;

    uint32_t pos = num_bits - 1;
    while(x != 0) {
        bits[pos] = (x & 1) + '0';
        x >>= 1;
        pos -= 1;
    }

    std::string ret{bits};
    delete[] bits;

    return ret;
}
