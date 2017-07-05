#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class AssemblerSimple_SingleInstruction_Test;

#include "utils.h"

#include "tokens.h"
#include "parser_gen/parser.hpp"

#include "printer.h"
#include "logger.h"

#include "state.h"

#include "instructions.h"
#include "instruction_encoder.h"

#include "assembler.h"

using namespace core;

Assembler::Assembler(bool log_enable, utils::IPrinter & printer) :
    logger(printer)
{
    this->log_enable = log_enable;
}

void Assembler::processInstruction(std::string const & filename, Token const * inst,
    uint32_t & encoded_instruction, std::map<std::string, uint32_t> const & labels) const
{
    std::vector<Instruction const *> candidates;
    bool valid_instruction = encoder.findInstruction(inst, candidates);

    if(valid_instruction) {
        encoder.encodeInstruction(log_enable, logger, filename, file_buffer[inst->row_num],
            candidates[0], inst, encoded_instruction, labels);

        if(log_enable) {
            logger.printf(PRINT_TYPE_DEBUG, true, "%s => %s",
                file_buffer[inst->row_num].c_str(), udecToBin(encoded_instruction, 16).c_str());
        }
    } else {
        if(candidates.size() == 0) {
            if(log_enable) {
                // this shouldn't happen, because if there are no candidates it should've been
                // retyped as a LABEL
                logger.printfMessage(PRINT_TYPE_ERROR, filename, inst,
                    file_buffer[inst->row_num], "\'%s\' is not a valid instruction",
                    inst->str.c_str());
                logger.newline();
            }
            throw std::runtime_error("could not find a valid instruction");
        } else {
            logger.printfMessage(PRINT_TYPE_ERROR, filename, inst,
                file_buffer[inst->row_num], "not a valid usage of \'%s\' instruction",
                inst->str.c_str());
            for(Instruction const * candidate : candidates) {
                logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'%s\'?", candidate->toFormatString().c_str());
            }
            logger.newline();

            throw std::runtime_error("matched instruction with a candidate, but some operands were incorrect");
        }
    }
}

// note: new_orig is untouched if the .orig is not valid
bool Assembler::setOrig(std::string const & filename, Token const * orig, uint32_t & new_orig)
{
    if(orig->checkPseudoType("orig")) {     // sanity check...
        if(orig->num_operands != 1) {
            if(log_enable) {
                logger.printfMessage(PRINT_TYPE_WARNING, filename, orig,
                    file_buffer[orig->row_num], "incorrect number of operands");
                logger.newline();
            }
            return false;
        } else {
            if(orig->opers->type != NUM) {
                if(log_enable) {
                    logger.printfMessage(PRINT_TYPE_WARNING, filename,
                        orig->opers, file_buffer[orig->row_num], "illegal operand");
                    logger.newline();
                }
                return false;
            } else {
                uint32_t value = ((uint32_t) orig->opers->num) & 0xffff;
                if((uint32_t) orig->opers->num > 0xffff) {
                    logger.printfMessage(PRINT_TYPE_WARNING, filename,
                        orig->opers, file_buffer[orig->row_num],
                        "truncating 0x%0.8x to 0x%0.4x", (uint32_t) orig->opers->num,
                        value);
                    logger.newline();
                }
                new_orig = value;
            }
        }
    }

    return true;
}

Token * Assembler::removeNewlineTokens(Token * program)
{
    Token * prev_state = nullptr;
    Token * cur_state = program;

    // remove newline tokens
    while(cur_state != nullptr) {
        bool delete_cur_state = false;
        if(cur_state->type == NEWLINE) {
            if(prev_state) {
                prev_state->next = cur_state->next;
            } else {
                // if we start off with newlines, move the program pointer forward
                program = cur_state->next;
            }
            delete_cur_state = true;
        } else {
            prev_state = cur_state;
        }

        Token * next_state = cur_state->next;
        if(delete_cur_state) {
            delete cur_state;
        }
        cur_state = next_state;
    }

    // since you may have moved the program pointer, you need to return it
    return program;
}

bool Assembler::findFirstOrig(std::string const & filename, Token * program,
    Token *& program_start, uint32_t & cur_orig)
{
    Token * cur_state = program;
    bool found_valid_orig = false;

    // find the orig
    while(cur_state != nullptr && ! found_valid_orig) {
        // ignore everything until you find the first orig
        while(cur_state != nullptr && ! cur_state->checkPseudoType("orig")) {
            // TODO: allow for exceptions, such as .external
            if(log_enable) {
                logger.xprintfMessage(PRINT_TYPE_WARNING, filename, 0,
                    file_buffer[cur_state->row_num].length(), cur_state,
                    file_buffer[cur_state->row_num], "ignoring statement before .orig");
                logger.newline();
            }
            cur_state = cur_state->next;
        }

        // looks like you hit nullptr before a .orig, meaning there is no .orig
        if(cur_state == nullptr) {
            if(log_enable) {
                logger.printf(PRINT_TYPE_ERROR, true, "no .orig found in \'%s\'", filename.c_str());
            }
            return false;
        }

        // check to see if .orig is valid
        // if so, stop looking; if not, move on and try again
        if(setOrig(filename, cur_state, cur_orig)) {
            found_valid_orig = true;
        }

        cur_state = cur_state->next;
    }

    // you hit nullptr after seeing at least one .orig, meaning there is no valid .orig
    if(! found_valid_orig) {
        if(log_enable) {
            logger.printf(PRINT_TYPE_ERROR, true, "no valid .orig found in \'%s\'", filename.c_str());
        }
        return false;
    }

    program_start = cur_state;

    return true;
}

void Assembler::processOperands(Token * inst)
{
    Token * oper = inst->opers;
    // redo operand types
    while(oper != nullptr) {
        if(oper->type == STRING) {
            if(encoder.findReg(oper->str)) {
                oper->type = OPER_TYPE_REG;
            } else {
                oper->type = OPER_TYPE_LABEL;
            }
        } else if(oper->type == NUM) {
            oper->type = OPER_TYPE_NUM;
        }

        oper = oper->next;
    }
}

void Assembler::separateLabels(std::string const & filename, Token * program)
{
    Token * cur_state = program;
    // since the parser can't distinguish between an instruction and a label by design,
    // we need to do it while analyzing the tokens using a simple rule: if the first INST
    // of a chain of tokens is not a valid instruction, assume it's a label
    while(cur_state != nullptr) {
        std::vector<Instruction const *> candidates;
        if((cur_state->type == INST && ! encoder.findInstruction(cur_state, candidates) &&
             candidates.size() == 0) || cur_state->type == LABEL)
        {
            cur_state->type = LABEL;
            if(cur_state->opers != nullptr) {
                Token * upgrade_state = cur_state->opers;
                // if there is something after the label that the parser marked as an operand
                if(upgrade_state->type == PSEUDO || encoder.findInstructionByName(upgrade_state->str)) {
                    // elevate the INST to a proper token in the chain
                    if(upgrade_state->type != PSEUDO) {
                        upgrade_state->type = INST;
                        upgrade_state->opers = upgrade_state->next;
                    }
                    upgrade_state->next = cur_state->next;
                    cur_state->next = upgrade_state;
                    cur_state->opers = nullptr;

                    uint32_t num_operands = 0;
                    Token * cur_oper = cur_state->next->opers;
                    while(cur_oper != nullptr) {
                        num_operands += 1;
                        cur_oper = cur_oper->next;
                    }
                    cur_state->next->num_operands = num_operands;
                } else {
                    if(log_enable) {
                        logger.printfMessage(PRINT_TYPE_ERROR, filename, cur_state,
                            file_buffer[cur_state->row_num],
                            "\'%s\' is being interpreted as a label, did you mean for it to be an instruction?",
                            cur_state->str.c_str());
                        logger.newline();
                    }
                }
            }
        }
        cur_state = cur_state->next;
    }
}

bool Assembler::processTokens(std::string const & filename, Token * program,
    std::map<std::string, uint32_t> & labels, Token *& program_start)
{
    program = removeNewlineTokens(program);
    separateLabels(filename, program);

    Token * temp = program;
    while(temp != nullptr) {
        //std::cout << *temp;
        temp = temp->next;
    }

    Token * cur_state;
    uint32_t cur_orig = 0;
    if(! findFirstOrig(filename, program, cur_state, cur_orig)) {
        return false;
    }

    program_start = cur_state;

    uint32_t pc_offset = 0;
    while(cur_state != nullptr) {
        // allow for multiple .orig in a single file
        // TODO: make this so that you have to have a .end and then another .orig
        if(cur_state->checkPseudoType("orig")) {
            if(! setOrig(filename, cur_state, cur_orig)) {
                if(log_enable) {
                    logger.printf(PRINT_TYPE_WARNING, true, "ignoring invalid .orig");
                }
            } else {
                pc_offset = 0;
            }
        }

        if(cur_state->type == LABEL) {
            std::string const & label = cur_state->str;

            auto search = labels.find(label);
            if(search != labels.end()) {
                if(log_enable) {
                    logger.printfMessage(PRINT_TYPE_WARNING, filename, cur_state,
                        file_buffer[cur_state->row_num], "redefining label \'%s\'",
                        cur_state->str.c_str());
                    logger.newline();
                }
            }

            labels[label] = cur_orig + pc_offset;

            if(log_enable) {
                logger.printf(PRINT_TYPE_DEBUG, true, "setting label \'%s\' to 0x%X",
                    label.c_str(), cur_orig + pc_offset);
            }
        }

        cur_state->pc = cur_orig + pc_offset;

        if(cur_state->type == INST) {
            processOperands(cur_state);
            pc_offset += 1;
        } else if(cur_state->type == PSEUDO) {
            // don't do any error checking, just ignore the pseduo op if it doesn't meet the requirements
            if(cur_state->str == "fill") {
                pc_offset += 1;
            } else if(cur_state->str == "stringz") {
                if(cur_state->opers != nullptr) {
                    Token * oper = cur_state->opers;
                    if(oper->type == STRING) {
                        pc_offset += oper->str.size() + 1;
                    } else if(oper->type == NUM) {
                        pc_offset += std::to_string(oper->num).size() + 1;
                    }
                }
            } else if(cur_state->str == "blkw") {
                if(cur_state->opers != nullptr) {
                    Token * oper = cur_state->opers;
                    if(oper->type == NUM) {
                        pc_offset += oper->num;
                    }
                }
            }
        }

        cur_state = cur_state->next;
    }

    return true;
}

void Assembler::processPseudo(std::string const & filename, Token const * inst,
    std::vector<uint32_t> & object_file,
    std::map<std::string, uint32_t> const & labels) const
{
    if(inst->str == "fill") {
        Token * oper = inst->opers;
        if(inst->num_operands != 1 || (oper->type != NUM && oper->type != STRING)) {
            logger.printfMessage(PRINT_TYPE_ERROR, filename, inst,
                file_buffer[inst->row_num], "not a valid usage of .fill pseudo-op");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.fill num\'?");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.fill label\'?");
            throw std::runtime_error("not a valid usage of .fill pseudo-op");
        } else {
            if(oper->type == NUM) {
                object_file.push_back(oper->num);
            } else if(oper->type == STRING) {
                auto search = labels.find(oper->str);
                if(search != labels.end()) {
                    object_file.push_back(search->second);
                } else {
                    logger.printfMessage(PRINT_TYPE_ERROR, filename, oper,
                        file_buffer[oper->row_num], "unknown label \'%s\'",
                        oper->str.c_str());
                    throw std::runtime_error("unknown label");
                }
            }
        }
    } else if(inst->str == "stringz") {
        Token * oper = inst->opers;
        if(inst->num_operands != 1 || (oper->type != NUM && oper->type != STRING)) {
            logger.printfMessage(PRINT_TYPE_ERROR, filename, inst,
                file_buffer[inst->row_num], "not a valid usage of .stringz pseudo-op");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.stringz string\'?");
            throw std::runtime_error("not a valid usage of .stringz pseudo-op");
        } else {
            std::string value;
            if(oper->type == NUM) {
                value = std::to_string(oper->num);
                logger.printfMessage(PRINT_TYPE_WARNING, filename, oper,
                    file_buffer[inst->row_num], "interpreting numeric value as decimal string \'%s\'", value.c_str());
                logger.printf(PRINT_TYPE_NOTE, false, "did you mean to put the numeric value in quotes?");
            } else if(oper->type == STRING) {
                value = oper->str;
            }

            for(char i : value) {
                object_file.push_back(((uint32_t) i) & 0xff);
            }
            object_file.push_back(0);
        }
    } else if(inst->str == "blkw") {
        Token * oper = inst->opers;
        if(inst->num_operands != 1 || oper->type != NUM) {
            logger.printfMessage(PRINT_TYPE_ERROR, filename, inst,
                file_buffer[inst->row_num], "not a valid usage of .blkw pseudo-op");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.blkw num\'?");
            throw std::runtime_error("not a valid usage of .blkw pseudo-op");
        } else {
            if(oper->type == NUM) {
                for(uint32_t i = 0; i < (uint32_t) oper->num; i += 1) {
                    object_file.push_back(0);
                }
            }
        }
    }
}

bool Assembler::assembleProgram(std::string const & filename, Token * program,
    std::map<std::string, uint32_t> & labels, std::vector<uint32_t> & object_file)
{
    std::ifstream file(filename);

    // load program into buffer for error messages
    if(file.is_open()) {
        std::string line;

        file_buffer.clear();
        while(std::getline(file, line)) {
            file_buffer.push_back(line);
        }

        file.close();
    } else {
        if(log_enable) {
            logger.printf(PRINT_TYPE_WARNING, true,
                "somehow the file got destroyed in the last couple of milliseconds, skipping file %s ...",
                filename.c_str());
        }

        return false;
    }

    if(log_enable) {
        logger.printf(PRINT_TYPE_INFO, true, "assembling \'%s\'", filename.c_str());
        logger.printf(PRINT_TYPE_INFO, true, "beginning first pass ...");
    }

    bool p1_success = true;
    Token * cur_state = nullptr;
    if(! processTokens(filename, program, labels, cur_state)) {
        if(log_enable) {
            logger.printf(PRINT_TYPE_ERROR, true, "first pass failed");
        }
        p1_success = false;
    } else {
        if(log_enable) {
            logger.printf(PRINT_TYPE_INFO, true, "first pass completed successfully, beginning second pass ...");
        }
    }

    bool p2_success = true;
    bool first = true;
    while(cur_state != nullptr) {
        if(first) {
            object_file.push_back(cur_state->pc);
            first = false;
        }

        if(cur_state->type == INST) {
            uint32_t encoded_instruction;

            try {
                processInstruction(filename, cur_state, encoded_instruction, labels);
                object_file.push_back(encoded_instruction);
            } catch(std::runtime_error & e) {
                p2_success = false;
            }
        } else if(cur_state->type == PSEUDO) {
            try {
                processPseudo(filename, cur_state, object_file, labels);
            } catch(std::runtime_error & e) {
                p2_success = false;
            }
        }
        cur_state = cur_state->next;
    }

    if(log_enable) {
        if(p2_success) {
            logger.printf(PRINT_TYPE_INFO, true, "second pass completed successfully");
        } else {
            logger.printf(PRINT_TYPE_ERROR, true, "second pass failed");
        }
    }

    return p1_success && p2_success;
}

extern FILE * yyin;
extern int yyparse(void);
extern Token * root;
extern int row_num, col_num;

void Assembler::genObjectFile(std::string const & filename)
{
    std::map<std::string, uint32_t> labels;
    std::vector<uint32_t> object_file;

    if((yyin = fopen(filename.c_str(), "r")) == nullptr) {
        logger.printf(PRINT_TYPE_WARNING, true, "skipping file %s ...", filename.c_str());
    } else {
        row_num = 0; col_num = 0;
        yyparse();
        bool status = assembleProgram(filename, root, labels, object_file);
        if(status) {
            char * data = new char[object_file.size() * 2];
            uint32_t pos = 0;

            for(auto i : object_file) {
                // output in big endian so it's easy to read in xxd
                data[pos] = (char) ((i & 0xff00) >> 8);
                ++pos;
                data[pos] = (char) (i & 0xff);
                ++pos;
                logger.printf(PRINT_TYPE_EXTRA, true, "0x%0.4x", i);
            }

            auto dot = filename.find_last_of('.');
            std::string obj_filename = filename.substr(0, dot) + ".obj";
            std::ofstream obj_file(obj_filename, std::ios::binary);
            obj_file.write(data, object_file.size() * 2);
            obj_file.close();
        }

        fclose(yyin);
    }
}
