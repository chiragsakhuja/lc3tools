#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

class AssemblerSimple_SingleInstruction_Test;

#include "assembler.h"
#include "instruction_encoder.h"
#include "logger.h"
#include "tokens.h"
#include "../common/printer.h"
#include "parser_gen/parser.hpp"

using namespace core;

Assembler::Assembler(bool log_enable, utils::Printer & printer)
{
    this->logger = new AssemblerLogger(printer);
    this->log_enable = log_enable;
    this->instructions = new InstructionGenerator();
    this->encoder = new InstructionEncoder(*instructions);
}

Assembler::~Assembler(void)
{
    delete logger;
    delete instructions;
    delete encoder;
}

void Assembler::processInstruction(std::string const & filename, Token const * inst,
    uint32_t & encoded_instruction, std::map<std::string, uint32_t> const & labels) const
{
    std::vector<Instruction *> candidates;
    bool valid_instruction = encoder->findInstruction(inst, candidates);

    if(valid_instruction) {
        encoder->encodeInstruction(log_enable, *logger, filename, file_buffer[inst->row_num],
            candidates[0], inst, encoded_instruction, labels);

        if(log_enable) {
            logger->printf(utils::PrintType::DEBUG, "%s => %s",
                file_buffer[inst->row_num].c_str(), udecToBin(encoded_instruction, 16).c_str());
        }
    } else {
        if(candidates.size() == 0) {
            if(log_enable) {
                // this shouldn't happen, because if there are no candidates it should've been
                // retyped as a LABEL
                logger->printfMessage(utils::PrintType::ERROR, filename, inst,
                    file_buffer[inst->row_num], "\'%s\' is not a valid instruction",
                    inst->str.c_str());
                logger->newline();
            }
            throw std::runtime_error("could not find a valid instruction");
        } else {
            logger->printfMessage(utils::PrintType::ERROR, filename, inst,
                file_buffer[inst->row_num], "not a valid usage of \'%s\' instruction",
                inst->str.c_str());
            for(Instruction * candidate : candidates) {
                std::stringstream assembly;
                assembly << candidate->name << " ";
                std::string prefix = "";
                for(Operand * operand : candidate->operands) {
                    if(operand->type != OPER_TYPE_FIXED) {
                        assembly << prefix << operand->type_str;
                        prefix = ", ";
                    }
                }
                logger->printf(utils::PrintType::NOTE, "did you mean \'%s\'?", assembly.str().c_str());
            }
            logger->newline();

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
                logger->printfMessage(utils::PrintType::WARNING, filename, orig,
                    file_buffer[orig->row_num], "incorrect number of operands");
                logger->newline();
            }
            return false;
        } else {
            if(orig->opers->type != NUM) {
                if(log_enable) {
                    logger->printfMessage(utils::PrintType::WARNING, filename,
                        orig->opers, file_buffer[orig->row_num], "illegal operand");
                    logger->newline();
                }
                return false;
            } else {
                uint32_t value = ((uint32_t) orig->opers->num) & 0xffff;
                if((uint32_t) orig->opers->num > 0xffff) {
                    logger->printfMessage(utils::PrintType::WARNING, filename,
                        orig->opers, file_buffer[orig->row_num],
                        "truncating 0x%0.8x to 0x%0.4x", (uint32_t) orig->opers->num,
                        value);
                    logger->newline();
                }
                new_orig = value;
            }
        }
    }

    return true;
}

bool Assembler::processPseudo(std::string const & filename, Token const * pseudo)
{
    if(pseudo->checkPseudoType("orig")) {
        uint32_t dummy;
        setOrig(filename, pseudo, dummy);
    } else if(pseudo->checkPseudoType("end")) {
        // do nothing
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
                logger->xprintfMessage(utils::PrintType::WARNING, filename, 0,
                    file_buffer[cur_state->row_num].length(), cur_state,
                    file_buffer[cur_state->row_num], "ignoring statement before .orig");
                logger->newline();
            }
            cur_state = cur_state->next;
        }

        // looks like you hit nullptr before a .orig, meaning there is no .orig
        if(cur_state == nullptr) {
            if(log_enable) {
                logger->printf(utils::PrintType::ERROR, "no .orig found in \'%s\'", filename.c_str());
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
            logger->printf(utils::PrintType::ERROR, "no valid .orig found in \'%s\'", filename.c_str());
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
            if(encoder->findReg(oper->str)) {
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

bool Assembler::processTokens(std::string const & filename, Token * program, Token *& program_start)
{
    program = removeNewlineTokens(program);
    Token * cur_state;
    uint32_t cur_orig = 0;
    if(! findFirstOrig(filename, program, cur_state, cur_orig)) {
        return false;
    }

    program_start = cur_state;

    uint32_t pc_offset = 0;
    while(cur_state != nullptr) {
        // allow for multiple .orig in a single file
        if(cur_state->checkPseudoType("orig")) {
            if(! setOrig(filename, cur_state, cur_orig)) {
                if(log_enable) {
                    logger->printf(utils::PrintType::WARNING, "ignoring invalid .orig");
                }
            } else {
                pc_offset = 0;
            }
        }

        std::vector<Instruction *> candidates;
        if(cur_state->type == LABEL ||
            (cur_state->type == INST && ! encoder->findInstruction(cur_state, candidates) &&
             candidates.size() == 0))
        {
            cur_state->type = LABEL;
            std::string const & label = cur_state->str;

            if(log_enable) {
                logger->printf(utils::PrintType::DEBUG, "setting label \'%s\' to 0x%X",
                    label.c_str(), cur_orig + pc_offset);
            }
        }

        cur_state->pc = cur_orig + pc_offset;

        if(cur_state->type == INST) {
            pc_offset += 1;
            processOperands(cur_state);
        }
        // TODO: account for block allocations (e.g. .fill, .stringz)

        cur_state = cur_state->next;
    }

    return true;
}

bool Assembler::assembleProgram(std::string const & filename, Token * program,
    std::map<std::string, uint32_t> & labels)
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
            logger->printf(utils::PrintType::WARNING,
                "somehow the file got destroyed in the last couple of milliseconds, skipping file %s ...",
                filename.c_str());
        }

        return false;
    }

    if(log_enable) {
        logger->printf(utils::PrintType::INFO, "assembling \'%s\'", filename.c_str());
        logger->printf(utils::PrintType::INFO, "beginning first pass ...");
    }

    Token * cur_state = nullptr;
    if(! processTokens(filename, program, cur_state)) {
        if(log_enable) {
            logger->printf(utils::PrintType::ERROR, "first pass failed");
        }
        return false;
    }

    if(log_enable) {
        logger->printf(utils::PrintType::INFO, "first pass completed successfully, beginning second pass ...");
    }

    while(cur_state != nullptr) {
        if(cur_state->type == INST) {
            uint32_t encoded_instruction;

            processInstruction(filename, cur_state, encoded_instruction, labels);
        }
        cur_state = cur_state->next;
    }

    if(log_enable) {
        logger->printf(utils::PrintType::INFO, "second pass completed successfully");
    }

    return true;
}

extern FILE *yyin;
extern int yyparse(void);
extern Token *root;
extern int row_num, col_num;

void Assembler::genObjectFile(char const * filename)
{
    std::map<std::string, uint32_t> labels;
    if((yyin = fopen(filename, "r")) == nullptr) {
         logger->printf(utils::PrintType::WARNING, "skipping file %s ...", filename);
    } else {
        row_num = 0; col_num = 0;
        yyparse();
        assembleProgram(filename, root, labels);

        fclose(yyin);
    }
}
