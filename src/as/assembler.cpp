#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <iostream>
#include <list>
#include <algorithm>
#include <cstdint>

class AssemblerSimple_SingleInstruction_Test;

#include "tokens.h"
#include "utils/printer.h"
#include "thirdparty/jsonxx/jsonxx.h"
#include "logger.h"
#include "instruction_encoder.h"
#include "parser.hpp"
#include "assembler.h"

Assembler::Assembler()
{
    sectionStart = 0;
}

bool Assembler::processInstruction(  bool log_enable, AssemblerLogger const & logger, InstructionEncoder & encoder
                                   , std::string const & filename, Token const * inst
                                   , std::map<std::string, int> const & symbol_table, uint32_t & encoded_instruction) const
{
    std::string const & op = inst->str;
    bool success = true;
    std::list<Instruction *> const & encs = encoder.insts[op];

    Instruction *potentialMatch = nullptr;
    bool foundMatch = false;

    // check all encodings to see if there is a match
    for(auto it = encs.begin(); it != encs.end(); it++) {
        // first make sure the number of operands is the same, otherwise it's a waste
        if((*it)->oper_types.size() == inst->num_operands) {
            potentialMatch = *it;
            bool actualMatch = true;
            const Token *curOper = inst->opers;

            // iterate through the oeprand types to see if the assembly matches
            for(auto it = potentialMatch->oper_types.begin(); it != potentialMatch->oper_types.end(); it++) {
                if(! (*it)->compareTypes(curOper->type)) {
                    actualMatch = false;
                    break;
                }

                curOper = curOper->next;
            }

            // found a match, stop searching
            if(actualMatch) {
                foundMatch = true;
                break;
            }
        }
    }

    if(! foundMatch) {
        std::cout << "No match\n";
        // if there was no match, check to see if it was because of incorrect number of operands or incorrect operands
        if(potentialMatch == nullptr) {
            // this will only be the case if there are no encodings with the same number of operands as the assembly line
            if(log_enable) {
                logger.printfMessage(utils::PrintType::ERROR, filename, inst, file_buffer[inst->row_num], "incorrect number of operands for instruction \'%s\'", inst->str.c_str());
            }
        } else {
            // this will only be the case if there is at least one encoding with the same number of operands as the assembly line
            // since there is still no match, this will assume you were trying to match against the last encoding in the list
            const Token *cur = inst->opers;

            // iterate through the assembly line to see which operands were incorrect and print errors
            if(log_enable) {
                for(auto it = potentialMatch->oper_types.begin(); it != potentialMatch->oper_types.end(); it++) {
                    if(! (*it)->compareTypes(cur->type)) {
                        logger.printfMessage(utils::PrintType::ERROR, filename, cur, file_buffer[inst->row_num], "incorrect operand");
                    }

                    cur = cur->next;
                }
            }
        }

        success = false;
    } else {
        // there was a match, so take that match and encode
        success &= encoder.encodeInstruction(log_enable, logger, potentialMatch, inst, encoded_instruction);
    }

    return success;
}

// note: newOrig is untouched if the .orig is not valid
bool Assembler::getOrig(bool log_enable, AssemblerLogger const & logger, const std::string& filename, const Token *orig, int& newOrig)
{
    if(orig->checkPseudoType("orig")) {     // sanity check...
        if(log_enable && orig->num_operands != 1) {
            logger.printfMessage(utils::PrintType::ERROR, filename, orig, file_buffer[orig->row_num], "incorrect number of operands");
            return false;
        } else {
            if(log_enable && orig->opers->type != NUM) {
                logger.printfMessage(utils::PrintType::ERROR, filename, orig->opers, file_buffer[orig->row_num], "illegal operand");
                return false;
            } else {
                newOrig = orig->opers->num;
            }
        }
    }

    return true;
}

bool Assembler::processPseudo(bool log_enable, AssemblerLogger const & logger, const std::string& filename, const Token *pseudo)
{
    if(pseudo->checkPseudoType("orig")) {
        getOrig(log_enable, logger, filename, pseudo, sectionStart);
    } else if(pseudo->checkPseudoType("end")) {
        // do nothing
    }

    return true;
}

bool Assembler::processTokens(  bool log_enable, AssemblerLogger const & logger, InstructionEncoder & encoder, std::string const & filename
                              , Token * program, std::map<std::string, int> & symbol_table, Token *& program_start)
{
    bool found_valid_orig = false;
    int cur_orig = 0;
    Token * cur_state = program;

    // find the orig
    while(cur_state != nullptr && ! found_valid_orig) {
        // move through the program until you find the first orig
        while(cur_state != nullptr && ! cur_state->checkPseudoType("orig")) {
            // TODO: allow for exceptions, such as .external
            if(log_enable) {
                logger.xprintfMessage(  utils::PrintType::WARNING, filename, 0, file_buffer[cur_state->row_num].length(), cur_state
                                      , file_buffer[cur_state->row_num], "ignoring statement before valid .orig");
            }
            cur_state = cur_state->next;
        }

        // looks like you hit nullptr before a .orig, meaning there is no .orig
        if(cur_state == nullptr) {
            if(log_enable) {
                logger.printf(utils::PrintType::ERROR, "no .orig found in \'%s\'", filename.c_str());
            }
            return false;
        }

        // check to see if .orig is valid
        // if so, stop looking; if not, move on and try again
        if(getOrig(log_enable, logger, filename, cur_state, cur_orig)) {
            found_valid_orig = true;
        }

        cur_state = cur_state->next;
    }

    // you hit nullptr after seeing at least one .orig, meaning there is no valid .orig
    if(! found_valid_orig) {
        if(log_enable) {
            logger.printf(utils::PrintType::ERROR, "no valid .orig found in \'%s\'", filename.c_str());
        }
        return false;
    }

    // write output
    program_start = cur_state;

    int pc_offset = 0;
    while(cur_state != nullptr) {
        if(cur_state->checkPseudoType("orig")) {
            if(! getOrig(log_enable, logger, filename, cur_state, cur_orig)) {
                if(log_enable) {
                    logger.printf(utils::PrintType::WARNING, "ignoring invalid .orig");
                }
            }
        }

        if(cur_state->type == LABEL) {
            std::string const & label = cur_state->str;

            if(log_enable) {
                logger.printf(utils::PrintType::DEBUG, "setting label \'%s\' to 0x%X", label.c_str(), cur_orig + pc_offset);
            }
        }

        cur_state->pc = pc_offset;

        if(cur_state->type == INST) {
            pc_offset += 1;

            Token * oper = cur_state->opers;
            while(oper != nullptr) {
                if(oper->type == STRING) {
                    bool reg_exists = false;

                    for(auto it = encoder.regs.begin(); it != encoder.regs.end(); ++it) {
                        if(*it == oper->str) {
                            reg_exists = true;
                            break;
                        }
                    }

                    if(reg_exists) {
                        oper->type = OPER_TYPE_REG;
                    } else {
                        oper->type = OPER_TYPE_LABEL;
                    }
                }

                oper = oper->next;
            }
        }
        // TODO: account for block allocations (e.g. .fill, .stringz)

        cur_state = cur_state->next;
    }

    return true;
}

// TODO: change return type to int and actually propagate
bool Assembler::assembleProgram(bool log_enable, utils::Printer const & printer, std::string const & filename, Token * program, std::map<std::string, int> & symbol_table)
{
    std::ifstream file(filename);
    AssemblerLogger assembler_printer(printer);
    InstructionEncoder encoder(log_enable, printer);
    bool success = true;

    // load program into buffer for error messages
    if(file.is_open()) {
        std::string line;

        file_buffer.clear();
        while(std::getline(file, line)) {
            file_buffer.push_back(line);
        }

        file.close();
    } else {
        return false;
    }

    if(log_enable) {
        assembler_printer.printf(utils::PrintType::INFO, "beginning first pass ...");
    }

    Token * cur_state = nullptr;
    if(! processTokens(log_enable, assembler_printer, encoder, filename, program, symbol_table, cur_state)) {
        return false;
    }

    if(log_enable) {
        assembler_printer.printf(utils::PrintType::INFO, "first pass completed successefully, beginning second pass ...");
    }

    while(cur_state != nullptr) {
        if(cur_state->checkPseudoType("orig")) {
            success &= processPseudo(log_enable, assembler_printer, filename, cur_state);
        }

        if(cur_state->type == INST) {
            uint32_t encoded_instruction;

            success &= processInstruction(log_enable, assembler_printer, encoder, filename, cur_state, symbol_table, encoded_instruction);
        }
        cur_state = cur_state->next;
    }

    if(success && log_enable) {
        assembler_printer.printf(utils::PrintType::INFO, "second pass completed successfully");
    }

    return success;
}
