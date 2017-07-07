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

extern FILE * yyin;
extern int yyparse(void);
extern Token * root;
extern int row_num, col_num;

void Assembler::assemble(std::string const & asm_filename, std::string const & obj_filename)
{
    std::map<std::string, uint32_t> symbols;
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
        std::vector<utils::ObjectFileStatement> object_file = assembleChain(root, symbols, logger);

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

std::vector<utils::ObjectFileStatement> Assembler::assembleChain(Token * program,
    std::map<std::string, uint32_t> & symbols, AssemblerLogger & logger)
{
    logger.printf(PRINT_TYPE_INFO, true, "beginning first pass ...");

    Token * program_start = nullptr;

    try {
        program_start = firstPass(program, symbols, logger);
    } catch(core::exception const & e) {
        logger.printf(PRINT_TYPE_ERROR, true, "first pass failed");
        throw e;
    }

    logger.printf(PRINT_TYPE_INFO, true, "first pass completed successfully, beginning second pass ...");

    std::vector<utils::ObjectFileStatement> ret;
    try {
        ret = secondPass(program_start, symbols, logger);
    } catch(core::exception const & e) {
        logger.printf(PRINT_TYPE_ERROR, true, "second pass failed");
        throw e;
    }

    logger.printf(PRINT_TYPE_INFO, true, "second pass completed successfully");

    return ret;
}

Token * Assembler::firstPass(Token * program, std::map<std::string, uint32_t> & symbols, AssemblerLogger & logger)
{
    // TODO: make sure we aren't leaking tokens by changing the program start
    Token * program_start = removeNewlineTokens(program);
    toLower(program_start);
    separateLabels(program_start, logger);
    program_start = findOrig(program_start, logger);
    processStatements(program_start, logger);
    saveSymbols(program_start, symbols, logger);

    Token * temp = program_start;
    while(temp != nullptr) {
        std::cout << *temp;
        temp = temp->next;
    }

    return program_start;
}

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
                    logger.printfMessage(PRINT_TYPE_WARNING, cur_tok,
                        "\'%s\' is being interpreted as a label, did you mean for it to be an instruction?",
                        cur_tok->str.c_str());
                    logger.newline();
                }
            }
        }
        cur_tok = cur_tok->next;
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
                logger.newline();
                throw core::exception("incorrect number of operands to .orig");
            }

            if(cur_tok->opers->type != NUM) {
                logger.printfMessage(PRINT_TYPE_ERROR, cur_tok->opers, "illegal operand");
                logger.newline();
                throw core::exception("illegal operand to .orig");
            }

            // TODO: use encode function
            uint32_t oper_val = (uint32_t) cur_tok->opers->num;
            uint32_t trunc_oper_val =((uint32_t) oper_val) & 0xffff;
            if(oper_val > 0xffff) {
                logger.printfMessage(PRINT_TYPE_WARNING, cur_tok->opers, "truncating 0x%0.8x to 0x%0.4x",
                    oper_val, trunc_oper_val);
                logger.newline();
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

// precondition: first token is valid .orig
void Assembler::processStatements(Token * program, AssemblerLogger & logger)
{
    uint32_t pc = program->pc;
    uint32_t pc_offset = 0;
    Token * cur_tok = program->next;
    while(cur_tok != nullptr) {
        cur_tok->pc = pc + pc_offset;

        // assign pc to opernads too
        Token * cur_oper = cur_tok->opers;
        while(cur_oper != nullptr) {
            cur_oper->pc = cur_tok->pc;
            cur_oper = cur_oper->next;
        }

        if(cur_tok->type == INST) {
            processInstOperands(cur_tok);
            pc_offset += 1;
        } else if(cur_tok->type == PSEUDO) {
            // don't do any error checking, just ignore the pseduo op if it doesn't meet the requirements
            // we'll halt the assembler anyway if there is something wrong
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
            } else if(cur_tok->str == "end") {
                break;
            }
        }
        cur_tok = cur_tok->next;
    }

    // TODO: delete remaining tokens
    uint32_t leftover_statement_count = 0;
    while(cur_tok != nullptr) {
        leftover_statement_count += 1;
        cur_tok = cur_tok->next;
    }

    if(leftover_statement_count > 0) {
        logger.printf(PRINT_TYPE_WARNING, true, "ignoring %d statements after .end", leftover_statement_count);
    }
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

void Assembler::saveSymbols(Token * program, std::map<std::string, uint32_t> & symbols, AssemblerLogger & logger)
{
    Token * cur_tok = program;
    while(cur_tok != nullptr) {
        if(cur_tok->type == LABEL) {
            std::string const & label = cur_tok->str;

            if(symbols.find(label) != symbols.end()) {
                logger.printfMessage(PRINT_TYPE_WARNING, cur_tok, "redefining label \'%s\'", cur_tok->str.c_str());
                logger.newline();
            }

            symbols[label] = cur_tok->pc;

            logger.printf(PRINT_TYPE_DEBUG, true, "setting label \'%s\' to 0x%X", label.c_str(), cur_tok->pc);
        }
        cur_tok = cur_tok->next;
    }
}

// precondition: first token is orig
std::vector<utils::ObjectFileStatement> Assembler::secondPass(Token * program,
    std::map<std::string, uint32_t> symbols, AssemblerLogger & logger)
{
    bool success = true;
    std::vector<utils::ObjectFileStatement> ret;
    ret.emplace_back(program->pc, true);

    Token * cur_tok = program->next;
    while(cur_tok != nullptr) {
        try {
            if(cur_tok->type == INST) {
                uint32_t encoded = encodeInstruction(cur_tok, symbols, logger);
                ret.emplace_back(encoded, false);
            } else if(cur_tok->type == PSEUDO) {
                std::vector<utils::ObjectFileStatement> encoded = encodePseudo(cur_tok, symbols, logger);
                ret.insert(ret.end(), encoded.begin(), encoded.end());
            }
        } catch(core::exception const & e) {
            success = false;
        }
        cur_tok = cur_tok->next;
    }

    if(! success) {
        throw core::exception("second pass failed");
    }

    return ret;
}

uint32_t Assembler::encodeInstruction(Token * inst, std::map<std::string, uint32_t> symbols,
    AssemblerLogger & logger)
{
    std::vector<IInstruction const *> candidates;
    if(encoder.findInstruction(inst, candidates)) {
        uint32_t encoding = encoder.encodeInstruction(candidates[0], inst, symbols, logger);
        logger.printf(PRINT_TYPE_DEBUG, true, "%s => %s", logger.asm_blob[inst->row_num].c_str(),
            udecToBin(encoding, 16).c_str());
        return encoding;
    }

    if(candidates.size() == 0) {
        // this shouldn't happen, because if there are no candidates it should've been retyped as a LABEL
        logger.printfMessage(PRINT_TYPE_ERROR, inst, "\'%s\' is not a valid instruction", inst->str.c_str());
        logger.newline();
        throw core::exception("could not find a valid candidate for instruction");
    }

    logger.printfMessage(PRINT_TYPE_ERROR, inst, "not a valid usage of \'%s\' instruction", inst->str.c_str());
    for(IInstruction const * candidate : candidates) {
        logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'%s\'?", candidate->toFormatString().c_str());
    }
    logger.newline();

    throw core::exception("matched instruction with a candidate, but some operands were incorrect");
}

std::vector<utils::ObjectFileStatement> Assembler::encodePseudo(Token * pseudo,
    std::map<std::string, uint32_t> symbols, AssemblerLogger & logger)
{
    std::vector<utils::ObjectFileStatement> ret;

    // TODO: is it worth making this like the instruction encoder?
    if(pseudo->str == "fill") {
        Token * oper = pseudo->opers;
        if(pseudo->num_opers != 1 || (oper->type != NUM && oper->type != STRING)) {
            logger.printfMessage(PRINT_TYPE_ERROR, pseudo, "not a valid usage of .fill pseudo-op");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.fill num\'?");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.fill label\'?");
            logger.newline();
            throw core::exception("not a valid usage of .fill pseudo-op");
        }

        if(oper->type == NUM) {
            ret.emplace_back(oper->num, false);
        } else if(oper->type == STRING) {
            auto search = symbols.find(oper->str);
            if(search != symbols.end()) {
                ret.emplace_back((uint32_t) search->second, false);
            } else {
                logger.printfMessage(PRINT_TYPE_ERROR, oper, "unknown label \'%s\'", oper->str.c_str());
                logger.newline();
                throw std::runtime_error("unknown label");
            }
        }
    } else if(pseudo->str == "stringz") {
        Token * oper = pseudo->opers;
        if(pseudo->num_opers != 1 || (oper->type != NUM && oper->type != STRING)) {
            logger.printfMessage(PRINT_TYPE_ERROR, pseudo, "not a valid usage of .stringz pseudo-op");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.stringz string\'?");
            logger.newline();
            throw std::runtime_error("not a valid usage of .stringz pseudo-op");
        }

        std::string value;
        if(oper->type == NUM) {
            value = std::to_string(oper->num);
            logger.printfMessage(PRINT_TYPE_WARNING, oper, "interpreting numeric value as decimal string \'%s\'",
                value.c_str());
            logger.newline();
        } else if(oper->type == STRING) {
            value = oper->str;
        }

        for(char i : value) {
            ret.emplace_back(((uint32_t) i) & 0xff, false);
        }
        ret.emplace_back(0U, false);
    } else if(pseudo->str == "blkw") {
        Token * oper = pseudo->opers;
        if(pseudo->num_opers != 1 || oper->type != NUM) {
            logger.printfMessage(PRINT_TYPE_ERROR, pseudo, "not a valid usage of .blkw pseudo-op");
            logger.printf(PRINT_TYPE_NOTE, false, "did you mean \'.blkw num\'?");
            logger.newline();
            throw std::runtime_error("not a valid usage of .blkw pseudo-op");
        }

        for(uint32_t i = 0; i < (uint32_t) oper->num; i += 1) {
            ret.emplace_back(0U, false);
        }
    }

    return ret;
}

/*

void Assembler::processPseudo(std::string const & filename, Token const * inst,
    std::vector<uint32_t> & object_file,
    std::map<std::string, uint32_t> const & symbols) const
{
    } else if(inst->str == "stringz") {
    } else if(inst->str == "blkw") {
    }
}
*/

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
