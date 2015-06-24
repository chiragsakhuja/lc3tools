#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <iostream>
#include <list>
#include <algorithm>

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

bool Assembler::assembleInstruction(const std::string& filename, const Token *inst)
{
    AssemblerPrinter& printer = AssemblerPrinter::getInstance();
    InstructionEncoder& encoder = InstructionEncoder::getInstance();

    std::string& op = *(inst->data.str);
    bool success = true;
    std::list<Instruction *>& encs = InstructionEncoder::insts[op];

    Instruction *potentialMatch = nullptr;
    bool foundMatch = false;

    const Token *curArg = inst->args;

    // check all encodings to see if there is a match
    for(auto it = encs.begin(); it != encs.end(); it++) {
        // first make sure the number of operands is the same, otherwise it's a waste
        if((*it)->argTypes.size() == inst->numOperands) {
            potentialMatch = *it;
            bool actualMatch = true;

            // iterate through the oeprand types to see if the assembly matches
            for(auto it = potentialMatch->argTypes.begin(); it != potentialMatch->argTypes.end(); it++) {
                if((*it)->type != curArg->type) {
                    actualMatch = false;
                    break;
                }

                curArg = curArg->next;
            }

            // found a match, stop searching
            if(actualMatch) {
                foundMatch = true;
                break;
            }
        }
    }

    if(!foundMatch) {
        // if there was no match, check to see if it was because of incorrect number of operands or incorrect operands
        if(potentialMatch == nullptr) {
            // this will only be the case if there are no encodings with the same number of operands as the assembly line
            printer.printAssemblyError(filename, inst, fileBuffer[inst->rowNum], "incorrect number of operands");
        } else {
            // this will only be the case if there is at least one encoding with the same number of operands as the assembly line
            // since there is still no match, this will assume you were trying to match against the last encoding in the list
            const Token *cur = inst->args;

            // iterate through the assembly line to see which arguments were incorrect and print errors
            for(auto it = potentialMatch->argTypes.begin(); it != potentialMatch->argTypes.end(); it++) {
                if((*it)->type != cur->type) {
                    printer.printAssemblyError(filename, cur, fileBuffer[inst->rowNum], "incorrect operand");
                }

                cur = cur->next;
            }
        }

        success = false;
    } else {
        // there was a match, so take that match and encode
    }

    return success;
}

// note: newOrig is untouched if the .orig is not valid
bool Assembler::getOrig(const std::string& filename, const Token *orig, bool printErrors, int& newOrig)
{
    const AssemblerPrinter& printer = AssemblerPrinter::getInstance();

    if(orig->checkPseudoType("orig")) {     // sanity check...
        if(printErrors && orig->numOperands != 1) {
            printer.printAssemblyError(filename, orig, fileBuffer[orig->rowNum], "incorrect number of operands");
            return false;
        } else {
            if(printErrors && orig->args->type != NUM) {
                printer.printAssemblyError(filename, orig->args, fileBuffer[orig->rowNum], "illegal operand");
                return false;
            } else {
                newOrig = orig->args->data.num;
            }
        }
    }

    return true;
}

bool Assembler::processPseudo(const std::string& filename, const Token *pseudo)
{
    if(pseudo->checkPseudoType("orig")) {
        getOrig(filename, pseudo, true, sectionStart);
    } else if(pseudo->checkPseudoType("end")) {
        // do nothing
    }

    return true;
}

bool Assembler::processStatement(const std::string& filename, const Token *state)
{
    if(state->type == INST) {
        return assembleInstruction(filename, state);
    } else if(state->type == PSEUDO) {
        return processPseudo(filename, state);
    } else if(state->type == LABEL) {
        // return injectSymbol(state);
    } else {
        // well this is awkward...
        return false;
    }

    return true;
}

bool Assembler::preprocessProgram(const std::string& filename, Token *program, std::map<std::string, int>& symbolTable, Token *& programStart)
{
    bool foundValidOrig = false;
    int curOrig = 0;
    Token *curState = program;
    const AssemblerPrinter& printer = AssemblerPrinter::getInstance();

    // find the orig
    while(curState != nullptr && !foundValidOrig) {
        // move through the program until you find the first orig
        while(curState != nullptr && !curState->checkPseudoType("orig")) {
            // TODO: allow for exceptions, such as .external
            printer.printAssemblyWarningX(filename, 0, fileBuffer[curState->rowNum].length(), curState, fileBuffer[curState->rowNum], "ignoring statement before valid .orig");
            curState = curState->next;
        }

        // looks like you hit nullptr before a .orig, meaning there is no .orig
        if(curState == nullptr) {
            printer.printError("no .orig found in \'%s\'", filename.c_str());
            return false;
        }

        // check to see if .orig is valid
        // if so, stop looking; if not, move on and try again
        if(getOrig(filename, curState, true, curOrig)) {
            foundValidOrig = true;
        }

        curState = curState->next;
    }

    // you hit nullptr after seeing at least one .orig, meaning there is no valid .orig
    if(!foundValidOrig) {
        printer.printError("no valid .orig found in \'%s\'", filename.c_str());
        return false;
    }

    // write output
    programStart = curState;

    int pcOffset = 0;
    while(curState != nullptr) {
        if(curState->checkPseudoType("orig")) {
            if(!getOrig(filename, curState, true, curOrig)) {
                printer.printWarning("ignoring invalid .orig");
            }
        }

        if(curState->type == LABEL) {
            const std::string& label = *curState->data.str;

            printer.printDebugInfo("setting label \'%s\' to 0x%X", label.c_str(), curOrig + pcOffset);
        }

        curState->pc = pcOffset;

        if(curState->type == INST) {
            pcOffset++;
        }
        // TODO: account for block allocations (e.g. .fill, .stringz)
        // TODO: process arguments

        curState = curState->next;
    }

    return true;
}

// precondition: filename exists
// note what happens to the program
bool Assembler::assembleProgram(const std::string& filename, Token *program, std::map<std::string, int> &symbolTable)
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

    printer.printInfo("beginning first pass ...");

    Token *curState = nullptr;
    if(!preprocessProgram(filename, program, symbolTable, curState)) {
        return false;
    }

    printer.printInfo("first pass completed successefully, beginning second pass ...");

    while(curState != nullptr) {
        if(curState->checkPseudoType("orig")) {
            success &= processPseudo(filename, curState);
        }

        if(curState->type == LABEL) {
        }
        success &= processStatement(filename, curState);
        curState = curState->next;
    }

    if(success) {
        printer.printInfo("second pass completed successfully");
    }

    return success;
}
