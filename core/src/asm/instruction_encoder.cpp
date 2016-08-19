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
#include "tokens.h"
#include "../common/printer.h"
#include "../thirdparty/jsonxx/jsonxx.h"

#include "../common/configured_paths.h"

Operand::Operand() : Operand(OPER_TYPE_UNKNOWN, 0, 0) { }

Operand::Operand(int type, int lo, int hi)
{
    this->type = type;
    this->lo = lo;
    this->hi = hi;
}

bool Operand::compareTypes(int other_type) const
{
    if(   (   (other_type == OPER_TYPE_LABEL || other_type == OPER_TYPE_UNTYPED_NUM)
           && (type == OPER_TYPE_IMM || type == OPER_TYPE_PCOFFS || type == OPER_TYPE_PCOFFU)
          )
       || (other_type == type)
      )
    {
        return true;
    }

    return false;
}

Instruction::Instruction(int width, bool setcc, std::string const & label)
{
    bit_types = new int[width];
    std::fill_n(bit_types, width, OPER_TYPE_UNKNOWN);

    this->setcc = setcc;
    this->label = label;
}

Instruction::~Instruction()
{
    delete[] bit_types;
    bit_types = nullptr;

    for(auto it = oper_types.begin(); it != oper_types.end(); ++it) {
        delete *it;
    }
}

InstructionEncoder::InstructionEncoder(bool print_enable, utils::Printer & printer)
{
    std::string res_path(GLOBAL_RES_PATH);

    std::ifstream file(res_path + "/encodings.json");

    if(file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();

        jsonxx::Array list;
        list.parse(buffer.str());

        for(size_t i = 0; i < list.size(); i += 1) {
            if(! list.has<jsonxx::Object>(i)) {
                if(print_enable) {
                    printer.printf(utils::PrintType::ERROR, "unknown encoding");
                }
                continue;
            }

            jsonxx::Object state = list.get<jsonxx::Object>(i);

            if(! state.has<jsonxx::String>("type") || ! state.has<jsonxx::Object>("data")) {
                if(print_enable) {
                    printer.printf(utils::PrintType::ERROR, "incorrect encoding");
                }
                continue;
            }

            jsonxx::String type = state.get<jsonxx::String>("type");
            jsonxx::Object data = state.get<jsonxx::Object>("data");

            // begin the giant parser
            if(type == "CONF") {    // start config
                if(! data.has<jsonxx::Number>("width")) {
                    if(print_enable) {
                        printer.printf(utils::PrintType::ERROR, "unspecified width in encoding");
                    }
                    continue;
                }

                jsonxx::Number width = data.get<jsonxx::Number>("width");
                reg_width = width;

                if(print_enable) {
                    printer.printf(utils::PrintType::DEBUG, "configuring %d-bit registers", reg_width);
                }
            } else if(type == "REGS") {     // end config, start regs
                if(! data.has<jsonxx::Array>("reglist")) {
                    if(print_enable) {
                        printer.printf(utils::PrintType::ERROR, "unspecified reglist in encoding");
                    }
                    continue;
                }

                jsonxx::Array reglist = data.get<jsonxx::Array>("reglist");
                std::stringstream output;
                output << "configuring registers ";

                for(size_t j = 0; j < reglist.size(); j += 1) {
                    if(! reglist.has<jsonxx::String>(j)) {
                        if(print_enable) {
                            printer.printf(utils::PrintType::ERROR, "unknown type in reglist");
                        }
                        continue;
                    }

                    jsonxx::String reg = reglist.get<jsonxx::String>(j);

                    regs.push_back(reglist.get<jsonxx::String>(j));
                    output << reg << " ";
                }


                if(print_enable) {
                    printer.printf(utils::PrintType::DEBUG, output.str().c_str());
                }
            } else if(type == "INST") {     // end regs, start inst
                if(! data.has<jsonxx::String>("label") || ! data.has<jsonxx::Boolean>("setcc") || ! data.has<jsonxx::Array>("enc")) {
                    if(print_enable) {
                        printer.printf(utils::PrintType::ERROR, "unspecified fields in instruction encoding");
                    }
                    continue;
                }

                jsonxx::String label = data.get<jsonxx::String>("label");
                jsonxx::Boolean setcc = data.get<jsonxx::Boolean>("setcc");
                jsonxx::Array enc = data.get<jsonxx::Array>("enc");

                std::list<Instruction *> * same_op_insts = nullptr;
                if(reg_width == 0) {
                    if(print_enable) {
                        printer.printf(utils::PrintType::WARNING, "unconfigured register width, assuming 32-bit");
                    }
                }

                Instruction *new_inst = new Instruction(reg_width, setcc, label);

                for(size_t j = 0; j < enc.size(); j += 1) {
                    if(! enc.has<jsonxx::Object>(j)) {
                        if(print_enable) {
                            printer.printf(utils::PrintType::ERROR, "unknown type in %s instruction encoding", label.c_str());
                        }
                        continue;
                    }

                    jsonxx::Object inst = enc.get<jsonxx::Object>(j);

                    if(! inst.has<jsonxx::Number>("hi") || ! inst.has<jsonxx::Number>("lo") || ! inst.has<jsonxx::String>("type") || ! inst.has<jsonxx::Object>("data")) {
                        if(print_enable) {
                            printer.printf(utils::PrintType::ERROR, "unspecified fields in %s instruction encoding", label.c_str());
                        }
                        continue;
                    }

                    jsonxx::Number hi = inst.get<jsonxx::Number>("hi");
                    jsonxx::Number lo = inst.get<jsonxx::Number>("lo");
                    jsonxx::String inst_type = inst.get<jsonxx::String>("type");
                    jsonxx::Object inst_data = inst.get<jsonxx::Object>("data");

                    if(inst_type == "OPCODE" || inst_type == "FIXED") {
                        if(! inst_data.has<jsonxx::String>("value")) {
                            if(print_enable) {
                                printer.printf(utils::PrintType::ERROR, "unspecified value in fixed encoding for %s instruction", label.c_str());
                            }
                            continue;
                        }

                        jsonxx::String value = inst_data.get<jsonxx::String>("value");

                        if(inst_type == "OPCODE") {
                            same_op_insts = &insts[label];
                        }

                        for(int k = (int) lo; k <= (int) hi; k += 1) {
                            new_inst->bit_types[k] = value[hi - k] - '0';
                        }
                    } else if(inst_type == "REG" || inst_type == "IMM" || inst_type == "PCOFFS" || inst_type == "PCOFFU") {
                        if(! inst_data.has<jsonxx::Number>("pos")) {
                            if(print_enable) {
                                printer.printf(utils::PrintType::ERROR, "unspecified pos in dynamic encoding for %s instruction", label.c_str());
                            }
                            continue;
                        }

                        jsonxx::Number pos = inst_data.get<jsonxx::Number>("pos");
                        int type = 0;

                        if(inst_type == "REG") {
                            type = OPER_TYPE_REG;
                        } else if(inst_type == "IMM") {
                            type = OPER_TYPE_IMM;
                        } else if(inst_type == "PCOFFS") {
                            type = OPER_TYPE_PCOFFS;
                        } else if(inst_type == "PCOFFU") {
                            type = OPER_TYPE_PCOFFU;
                        }

                        new_inst->oper_types.insert(new_inst->oper_types.begin() + ((int) pos), new Operand(type, lo, hi));
                        std::fill_n(new_inst->bit_types + (int) lo, (int) (hi - lo + 1), type);
                    } else {
                        if(print_enable) {
                            printer.printf(utils::PrintType::ERROR, "unknown encoding type %s", type.c_str());
                        }
                        continue;
                    }
                }

                same_op_insts->push_back(new_inst);
            }
        }

        for(auto it = insts.begin(); it != insts.end(); ++it) {
            std::list<Instruction *>& encs = it->second;

            for(auto it2 = encs.begin(); it2 != encs.end(); ++it2) {
                std::stringstream output;
                Instruction * inst = *it2;

                output << inst->label << ": { ";

                for(int i = reg_width - 1; i >= 0; i -= 1) {
                    output << inst->bit_types[i];
                }

                output << " }";
                if(print_enable) {
                    printer.printf(utils::PrintType::DEBUG, "configuring instruction %s", output.str().c_str());
                }
            }
        }

        file.close();
    } else {
        if(print_enable) {
            printer.printf(utils::PrintType::ERROR, "could not open encodings.json");
        }
    }
}

InstructionEncoder::~InstructionEncoder()
{
    for(auto map_it = insts.begin(); map_it != insts.end(); ++map_it) {
        std::list<Instruction *> & encs = map_it->second;

        for(auto list_it = encs.begin(); list_it != encs.end(); ++list_it) {
            delete *list_it;
        }
    }

    insts.clear();
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
