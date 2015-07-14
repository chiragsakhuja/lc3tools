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
#include "instruction_encoder.h"
#include "assembler_printer.h"
#include "parser.hpp"
#include "assembler.h"

std::vector<std::string> Assembler::fileBuffer;
int Assembler::sectionStart;

Assembler& Assembler::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static Assembler instance;

    return instance;
}

Assembler::Assembler()
{
    sectionStart = 0;
}

bool Assembler::processInstruction(bool printEnable, const std::string& filename, const Token *inst, const std::map<std::string, int>& symbolTable, uint32_t& encodedInstruction)
{
    AssemblerPrinter& printer = AssemblerPrinter::getInstance();
    InstructionEncoder& encoder = InstructionEncoder::getInstance(printEnable);

    const std::string& op = inst->str;
    bool success = true;
    std::list<Instruction *>& encs = InstructionEncoder::insts[op];

    Instruction *potentialMatch = nullptr;
    bool foundMatch = false;

    // check all encodings to see if there is a match
    for(auto it = encs.begin(); it != encs.end(); it++) {
        // first make sure the number of operands is the same, otherwise it's a waste
        if((*it)->operTypes.size() == inst->numOperands) {
            potentialMatch = *it;
            bool actualMatch = true;
            const Token *curOper = inst->opers;

            // iterate through the oeprand types to see if the assembly matches
            for(auto it = potentialMatch->operTypes.begin(); it != potentialMatch->operTypes.end(); it++) {
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
            if(printEnable) {
                printer.printfAssemblyMessage(AssemblerPrinter::ERROR, filename, inst, fileBuffer[inst->rowNum], "incorrect number of operands for instruction \'%s\'", inst->str.c_str());
            }
        } else {
            // this will only be the case if there is at least one encoding with the same number of operands as the assembly line
            // since there is still no match, this will assume you were trying to match against the last encoding in the list
            const Token *cur = inst->opers;

            // iterate through the assembly line to see which operands were incorrect and print errors
            if(printEnable) {
                for(auto it = potentialMatch->operTypes.begin(); it != potentialMatch->operTypes.end(); it++) {
                    if(! (*it)->compareTypes(cur->type)) {
                        printer.printfAssemblyMessage(AssemblerPrinter::ERROR, filename, cur, fileBuffer[inst->rowNum], "incorrect operand");
                    }

                    cur = cur->next;
                }
            }
        }

        success = false;
    } else {
        // there was a match, so take that match and encode
        success &= encoder.encodeInstruction(printEnable, potentialMatch, inst, encodedInstruction);
    }

    return success;
}

// note: newOrig is untouched if the .orig is not valid
bool Assembler::getOrig(bool printEnable, const std::string& filename, const Token *orig, int& newOrig)
{
    const AssemblerPrinter& printer = AssemblerPrinter::getInstance();

    if(orig->checkPseudoType("orig")) {     // sanity check...
        if(printEnable && orig->numOperands != 1) {
            printer.printfAssemblyMessage(AssemblerPrinter::ERROR, filename, orig, fileBuffer[orig->rowNum], "incorrect number of operands");
            return false;
        } else {
            if(printEnable && orig->opers->type != NUM) {
                printer.printfAssemblyMessage(AssemblerPrinter::ERROR, filename, orig->opers, fileBuffer[orig->rowNum], "illegal operand");
                return false;
            } else {
                newOrig = orig->opers->num;
            }
        }
    }

    return true;
}

bool Assembler::processPseudo(bool printEnable, const std::string& filename, const Token *pseudo)
{
    if(pseudo->checkPseudoType("orig")) {
        getOrig(printEnable, filename, pseudo, sectionStart);
    } else if(pseudo->checkPseudoType("end")) {
        // do nothing
    }

    return true;
}

// TODO: explain what this does
bool Assembler::preprocessProgram(bool printEnable, const std::string& filename, Token *program, std::map<std::string, int>& symbolTable, Token *& programStart)
{
    bool foundValidOrig = false;
    int curOrig = 0;
    Token *curState = program;
    const AssemblerPrinter& printer = AssemblerPrinter::getInstance();
    const InstructionEncoder& encoder = InstructionEncoder::getInstance(printEnable);

    // find the orig
    while(curState != nullptr && ! foundValidOrig) {
        // move through the program until you find the first orig
        while(curState != nullptr && ! curState->checkPseudoType("orig")) {
            // TODO: allow for exceptions, such as .external
            if(printEnable) {
                printer.xprintfAssemblyMessage(AssemblerPrinter::WARNING, filename, 0, fileBuffer[curState->rowNum].length(), curState, fileBuffer[curState->rowNum], "ignoring statement before valid .orig");
            }
            curState = curState->next;
        }

        // looks like you hit nullptr before a .orig, meaning there is no .orig
        if(curState == nullptr) {
            if(printEnable) {
                printer.printfMessage(AssemblerPrinter::ERROR, "no .orig found in \'%s\'", filename.c_str());
            }
            return false;
        }

        // check to see if .orig is valid
        // if so, stop looking; if not, move on and try again
        if(getOrig(printEnable, filename, curState, curOrig)) {
            foundValidOrig = true;
        }

        curState = curState->next;
    }

    // you hit nullptr after seeing at least one .orig, meaning there is no valid .orig
    if(! foundValidOrig) {
        if(printEnable) {
            printer.printfMessage(AssemblerPrinter::ERROR, "no valid .orig found in \'%s\'", filename.c_str());
        }
        return false;
    }

    // write output
    programStart = curState;

    int pcOffset = 0;
    while(curState != nullptr) {
        if(curState->checkPseudoType("orig")) {
            if(! getOrig(printEnable, filename, curState, curOrig)) {
                if(printEnable) {
                    printer.printfMessage(AssemblerPrinter::WARNING, "ignoring invalid .orig");
                }
            }
        }

        if(curState->type == LABEL) {
            const std::string& label = curState->str;

            if(printEnable) {
                printer.printfMessage(AssemblerPrinter::DEBUG, "setting label \'%s\' to 0x%X", label.c_str(), curOrig + pcOffset);
            }
        }

        curState->pc = pcOffset;

        if(curState->type == INST) {
            pcOffset++;

            Token *oper = curState->opers;
            while(oper != nullptr) {
                if(oper->type == STRING) {
                    bool regExists = false;

                    for(auto it = encoder.regs.begin(); it != encoder.regs.end(); it++) {
                        if(*it == oper->str) {
                            regExists = true;
                            break;
                        }
                    }

                    if(regExists) {
                        oper->type = OPER_TYPE_REG;
                    } else {
                        oper->type = OPER_TYPE_LABEL;
                    }
                }

                oper = oper->next;
            }
        }
        // TODO: account for block allocations (e.g. .fill, .stringz)

        curState = curState->next;
    }

    return true;
}

// precondition: filename exists
// note what happens to the program
// TODO: change return type to int and actually propagate
bool Assembler::assembleProgram(bool printEnable, const std::string& filename, Token *program, std::map<std::string, int> &symbolTable)
{
    std::ifstream file(filename);
    const AssemblerPrinter& printer = AssemblerPrinter::getInstance();
    bool success = true;

    // load program into buffer for error messages
    if(file.is_open()) {
        std::string line;

        fileBuffer.clear();
        while(std::getline(file, line)) {
            fileBuffer.push_back(line);
        }

        file.close();
    } else {
        return false;
    }

    if(printEnable) {
        printer.printfMessage(AssemblerPrinter::INFO, "beginning first pass ...");
    }

    Token *curState = nullptr;
    if(! preprocessProgram(printEnable, filename, program, symbolTable, curState)) {
        return false;
    }

    if(printEnable) {
        printer.printfMessage(AssemblerPrinter::INFO, "first pass completed successefully, beginning second pass ...");
    }

    while(curState != nullptr) {
        if(curState->checkPseudoType("orig")) {
            success &= processPseudo(printEnable, filename, curState);
        }

        if(curState->type == INST) {
            uint32_t encodedInstruction;

            success &= processInstruction(printEnable, filename, curState, symbolTable, encodedInstruction);
        }
        curState = curState->next;
    }

    if(success && printEnable) {
        printer.printfMessage(AssemblerPrinter::INFO, "second pass completed successfully");
    }

    return success;
}
