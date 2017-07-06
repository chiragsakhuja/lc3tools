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

#include "utils.h"

#include "tokens.h"
#include "parser_gen/parser.hpp"

#include "printer.h"
#include "logger.h"

#include "files.h"

#include "state.h"

#include "instructions.h"
#include "instruction_encoder.h"

#include "assembler.h"

using namespace core;

/*
void Assembler::processInstruction(std::string const & filename, Token const * inst,
    uint32_t & encoded_instruction, std::map<std::string, uint32_t> const & labels) const
{
    std::vector<IInstruction const *> candidates;
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
            for(IInstruction const * candidate : candidates) {
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
        if(orig->num_opers != 1) {
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
}

void Assembler::separateLabels(std::string const & filename, Token * program)
{
    Token * cur_state = program;
    // since the parser can't distinguish between an instruction and a label by design,
    // we need to do it while analyzing the tokens using a simple rule: if the first INST
    // of a chain of tokens is not a valid instruction, assume it's a label
    while(cur_state != nullptr) {
        std::vector<IInstruction const *> candidates;
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

                    uint32_t num_opers = 0;
                    Token * cur_oper = cur_state->next->opers;
                    while(cur_oper != nullptr) {
                        num_opers += 1;
                        cur_oper = cur_oper->next;
                    }
                    cur_state->next->num_opers = num_opers;
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
    Token * temp = program;
    while(temp != nullptr) {
        std::cout << *temp;
        temp = temp->next;
    }

    program = removeNewlineTokens(program);
    separateLabels(filename, program);

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
                        std::stringstream new_str;
                        std::string value = oper->str;

                        if(value[0] == '"') {
                            if(value[value.size() - 1] == '"') {
                                value = value.substr(1, value.size() - 2);
                            } else {
                                value = value.substr(1);
                            }
                        }

                        for(uint32_t i = 0; i < value.size(); i += 1) {
                            char char_value = value[i];
                            if(char_value == '\\' && i + 1 < value.size()) {
                                if(value[i + 1] == 'n') {
                                    char_value = '\n';
                                }
                                i += 1;
                            }
                            new_str << char_value;
                        }
                        oper->str = new_str.str();

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
        if(inst->num_opers != 1 || (oper->type != NUM && oper->type != STRING)) {
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
        if(inst->num_opers != 1 || (oper->type != NUM && oper->type != STRING)) {
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
        if(inst->num_opers != 1 || oper->type != NUM) {
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
*/

Token * Assembler::removeNewlineTokens(Token * program)
{
    Token * program_start = program;
    Token * prev_tok = nullptr;
    Token * cur_tok = program;

    // remove newline toks
    while(cur_tok != nullptr) {
        bool del_cur_tok = false;
        if(cur_tok->type == NEWLINE) {
            if(prev_tok != nullptr) {
                prev_tok->next = cur_tok->next;
            } else {
                // if we start off with newlines, move the program pointer forward
                program_start = cur_tok->next;
            }
            del_cur_tok = true;
        } else {
            prev_tok = cur_tok;
        }

        Token * next_tok = cur_tok->next;
        if(del_cur_tok) {
            delete cur_tok;
        }
        cur_tok = next_tok;
    }

    // since you may have moved the program pointer, you need to return it
    return program_start;
}

void Assembler::separateLabels(Token * program, AssemblerLogger & logger)
{
    Token * cur_tok = program;
    // since the parser can't distinguish between an instruction and a label by design,
    // we need to do it while analyzing the tokens using a simple rule: if the first INST
    // of a chain of tokens is not a valid instruction, assume it's a label
    while(cur_tok != nullptr) {
        std::vector<IInstruction const *> candidates;
        // if the token was labeled an instruction and it's not even a candidate for an instruction,
        // assume it's a label
        if((cur_tok->type == INST && ! encoder.findInstruction(cur_tok, candidates) &&
                candidates.size() == 0) ||
            cur_tok->type == LABEL)
        {
            cur_tok->type = LABEL;
            if(cur_tok->opers != nullptr) {
                Token * upgrade_tok = cur_tok->opers;
                // if there is something after the label that the parser marked as an operand
                if(upgrade_tok->type == PSEUDO || encoder.findInstructionByName(upgrade_tok->str)) {
                    if(upgrade_tok->type != PSEUDO) {
                        upgrade_tok->type = INST;
                        // if it is a pseduo-op, then the opers are already correct because of parser behavior
                        // if not, opers is a nullptr because the opers are really in the next pointer
                        upgrade_tok->opers = upgrade_tok->next;
                        // recount operands (could just subtract 1, but do this just in case)
                        uint32_t num_opers = 0;
                        Token * cur_oper = upgrade_tok->opers;
                        while(cur_oper != nullptr) {
                            num_opers += 1;
                            cur_oper = cur_oper->next;
                        }
                        upgrade_tok->num_opers = num_opers;
                    }
                    // elevate the token to a proper token in the chain
                    upgrade_tok->next = cur_tok->next;
                    cur_tok->next = upgrade_tok;
                    cur_tok->opers = nullptr;
                    cur_tok->num_opers = 0;
                } else {
                    logger.printfMessage(PRINT_TYPE_ERROR, cur_tok,
                        "\'%s\' is being interpreted as a label, did you mean for it to be an instruction?",
                        cur_tok->str.c_str());
                }
            }
        }
        cur_tok = cur_tok->next;
    }
}

void Assembler::toLower(Token * token_chain)
{
    Token * cur_token = token_chain;
    while(cur_token != nullptr) {
        if(cur_token->type != NUM) {
            std::string & cur_str = cur_token->str;
            std::transform(cur_str.begin(), cur_str.end(), cur_str.begin(), ::tolower);
            if(! cur_token->checkPseudoType("stringz")) {
                toLower(cur_token->opers);
            }
        }
        cur_token = cur_token->next;
    }
}

Token * Assembler::findOrig(Token * program, AssemblerLogger & logger)
{
    Token * program_start = program;
    Token * cur_tok = program;
    bool found_valid_orig = false;
    uint32_t invalid_statement_count = 0;
    while(! found_valid_orig && cur_tok != nullptr) {
        if(! cur_tok->checkPseudoType("orig")) {
            if(cur_tok->type != LABEL) {
                invalid_statement_count += 1;
            }
        } else {
            found_valid_orig = true;
            if(cur_tok->num_opers != 1) {
                logger.printfMessage(PRINT_TYPE_ERROR, cur_tok, "incorrect number of operands");
                throw core::exception("incorrect number of operands to .orig");
            }

            if(cur_tok->opers->type != NUM) {
                logger.printfMessage(PRINT_TYPE_ERROR, cur_tok->opers, "illegal operand");
                throw core::exception("illegal operand to .orig");
            }

            // TODO: use encode function
            uint32_t oper_val = (uint32_t) cur_tok->opers->num;
            uint32_t trunc_oper_val =((uint32_t) oper_val) & 0xffff;
            if(oper_val > 0xffff) {
                logger.printfMessage(PRINT_TYPE_WARNING, cur_tok->opers, "truncating 0x%0.8x to 0x%0.4x",
                    oper_val, trunc_oper_val);
            }
            cur_tok->pc = trunc_oper_val;
            program_start = cur_tok;
        } 
        cur_tok = cur_tok->next;
    }

    if(! found_valid_orig) {
        logger.printf(PRINT_TYPE_ERROR, true, "could not find valid .orig in program");
        throw core::exception("could not find valid .orig");
    } else {
        if(invalid_statement_count > 0) {
            logger.printf(PRINT_TYPE_WARNING, true, "ignoring %d statements before .orig", invalid_statement_count);
        }
    }

    return program_start;
}

void Assembler::processInstOperands(Token * inst)
{
    Token * oper = inst->opers;
    // reassign operand types
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

void Assembler::processStringzOperands(Token * stringz)
{
    Token * oper = stringz->opers;
    if(oper->type == STRING) {
        std::stringstream new_str;
        std::string value = oper->str;

        if(value[0] == '"') {
            if(value[value.size() - 1] == '"') {
                value = value.substr(1, value.size() - 2);
            } else {
                value = value.substr(1);
            }
        }

        for(uint32_t i = 0; i < value.size(); i += 1) {
            char char_value = value[i];
            if(char_value == '\\' && i + 1 < value.size()) {
                if(value[i + 1] == 'n') {
                    char_value = '\n';
                }
                i += 1;
            }
            new_str << char_value;
        }
        oper->str = new_str.str();
    } else {
        oper->type = STRING;
        oper->str = std::to_string(oper->num);
    }
}

// precondition: first token is valid .orig
void Assembler::processStatements(Token * program)
{
    uint32_t pc = program->pc;
    uint32_t pc_offset = 0;
    Token * cur_tok = program->next;
    while(cur_tok != nullptr) {
        cur_tok->pc = pc + pc_offset;
        if(cur_tok->type == INST) {
            processInstOperands(cur_tok);
            pc_offset += 1;
        } else if(cur_tok->type == PSEUDO) {
            // don't do any error checking, just ignore the pseduo op if it doesn't meet the requirements
            if(cur_tok->str == "fill") {
                if(cur_tok->num_opers == 1) {
                    pc_offset += 1;
                }
            } else if(cur_tok->str == "stringz") {
                if(cur_tok->num_opers == 1) {
                    processStringzOperands(cur_tok);
                    pc_offset += cur_tok->opers->str.size() + 1;
                }
            } else if(cur_tok->str == "blkw") {
                if(cur_tok->num_opers == 1 && cur_tok->opers->type == NUM) {
                    pc_offset += cur_tok->opers->num;
                }
            }
        }
        cur_tok = cur_tok->next;
    }
}

void Assembler::saveSymbols(Token * program, std::map<std::string, uint32_t> & labels, AssemblerLogger & logger)
{
    Token * cur_tok = program;
    while(cur_tok != nullptr) {
        if(cur_tok->type == LABEL) {
            std::string const & label = cur_tok->str;

            if(labels.find(label) != labels.end()) {
                logger.printfMessage(PRINT_TYPE_WARNING, cur_tok, "redefining label \'%s\'", cur_tok->str.c_str());
            }

            labels[label] = cur_tok->pc;

            logger.printf(PRINT_TYPE_DEBUG, true, "setting label \'%s\' to 0x%X", label.c_str(), cur_tok->pc);
        }
        cur_tok = cur_tok->next;
    }
}

Token * Assembler::firstPass(Token * program, std::map<std::string, uint32_t> & labels, AssemblerLogger & logger)
{
    // TODO: make sure we aren't leaking tokens by changing the program start
    Token * program_start = removeNewlineTokens(program);
    toLower(program_start);
    separateLabels(program_start, logger);
    program_start = findOrig(program_start, logger);
    processStatements(program_start);
    saveSymbols(program_start, labels, logger);
    
    Token * temp = program_start;
    while(temp != nullptr) {
        std::cout << *temp;
        temp = temp->next;
    }

    return program_start;

    // TODO: everything following here
    /*

    program = removeNewlineTokens(program);
    separateLabels(filename, program);

    Token * cur_state;
    uint32_t cur_orig = 0;
    if(! findFirstOrig(filename, program, cur_state, cur_orig)) {
        return false;
    }

    program_start = cur_state;

    uint32_t pc_offset = 0;
    while(cur_state != nullptr) {

        if(cur_state->type == LABEL) {
        }

        cur_state->pc = cur_orig + pc_offset;

        if(cur_state->type == INST) {
            processOperands(cur_state);
            pc_offset += 1;
        } else if(cur_state->type == PSEUDO) {
        }

        cur_state = cur_state->next;
    }

    return true;
    */
}

std::vector<utils::ObjectFileStatement> Assembler::assembleChain(Token * program,
    std::map<std::string, uint32_t> & labels, AssemblerLogger & logger)
{
    logger.printf(PRINT_TYPE_INFO, true, "beginning first pass ...");

    try {
        Token * program_start = firstPass(program, labels, logger);
    } catch(core::exception const & e) {
        logger.printf(PRINT_TYPE_ERROR, true, "first pass failed");
        throw e;
    }

    logger.printf(PRINT_TYPE_INFO, true, "first pass completed successfully, beginning second pass ...");

    std::vector<utils::ObjectFileStatement> ret;
    return ret;
}

// assumes the file is valid
std::vector<std::string> Assembler::readFile(std::string const & filename)
{
    std::vector<std::string> file_buffer;
    std::ifstream file(filename);

    // load program into buffer for error messages
    if(file.is_open()) {
        std::string line;

        file_buffer.clear();
        while(std::getline(file, line)) {
            file_buffer.push_back(line);
        }

        file.close();
    }

    return file_buffer;
}

extern FILE * yyin;
extern int yyparse(void);
extern Token * root;
extern int row_num, col_num;

void Assembler::assemble(std::string const & asm_filename, std::string const & obj_filename)
{
    std::map<std::string, uint32_t> labels;
    AssemblerLogger logger(log_enable, printer);

    if((yyin = fopen(asm_filename.c_str(), "r")) == nullptr) {
        logger.printf(PRINT_TYPE_WARNING, true, "skipping file %s ...", asm_filename.c_str());
    } else {
        row_num = 0;
        col_num = 0;
        yyparse();

        logger.printf(PRINT_TYPE_INFO, true, "assembling \'%s\' into \'%s\'", asm_filename.c_str(),
            obj_filename.c_str());

        logger.filename = asm_filename;
        logger.asm_blob = readFile(asm_filename);
        std::vector<utils::ObjectFileStatement> object_file = assembleChain(root, labels, logger);

/*
 *        ObjectFileWriter writer(obj_filename);
 *        for(ObjectFileStatement const & state : object_file) {
 *            writer.writeStatement(state);
 *        }
 *
 */
        fclose(yyin);
    }
}

