#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <iostream>

#include "tokens.h"
#include "../utils/printer.h"
#include "assembler_printer.h"
#include "instruction.h"
#include "parser.hpp"
#include "assembler.h"

std::vector<std::string> Assembler::fileBuffer;
std::map<std::string, Instruction> Assembler::opcodeLUT;

Assembler& Assembler::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static Assembler instance;

    return instance;
}

Assembler::Assembler()
{
    opcodeLUT["add"]  = Instruction("add" , 3, 0x1);
    opcodeLUT["and"]  = Instruction("and" , 3, 0x5);
    opcodeLUT["br"]   = Instruction("br"  , 1, 0x0);
    opcodeLUT["jmp"]  = Instruction("jmp" , 1, 0xc);
    opcodeLUT["jsr"]  = Instruction("jsr" , 1, 0x4);
    opcodeLUT["jsrr"] = Instruction("jsrr", 3, 0x4);
    opcodeLUT["ld"]   = Instruction("ld"  , 2, 0x2);
    opcodeLUT["ldi"]  = Instruction("ldi" , 2, 0xa);
    opcodeLUT["ldr"]  = Instruction("ldr" , 3, 0x6);
    opcodeLUT["lea"]  = Instruction("lea" , 2, 0xe);
    opcodeLUT["not"]  = Instruction("not" , 2, 0x9);
    opcodeLUT["ret"]  = Instruction("ret" , 0, 0xc);
    opcodeLUT["rti"]  = Instruction("rti" , 0, 0x8);
    opcodeLUT["st"]   = Instruction("st"  , 2, 0x3);
    opcodeLUT["sti"]  = Instruction("sti" , 2, 0xd);
    opcodeLUT["str"]  = Instruction("str" , 3, 0x7);
    opcodeLUT["trap"] = Instruction("trap", 1, 0xf);
}

bool Assembler::assembleInstruction(const std::string& filename, const Token *inst)
{
    AssemblerPrinter &printer = AssemblerPrinter::getInstance();

    std::string& op = *(inst->data.str);
    bool brBits[3] = {false, false, false};     // 0:n, 1:z, 2:p
    bool success = true;
    Instruction &instObj = opcodeLUT["br"];     // default to branch

    if(op.find("br") == 0) {
        // starts with br (brn, brz, etc.)

        if(op == "br") {
            // br means always branch
            brBits[0] = true;
            brBits[1] = true;
            brBits[2] = true;
        } else {
            for(std::size_t i = 2; i < op.length(); i++) {
                int pos = inst->colNum + (int) i;
                if(op.at(i) == 'n') {
                    if(brBits[0]) {
                        printer.printAssemblyErrorX(filename, pos, 1, inst, fileBuffer[inst->rowNum], "duplicate condition in branch");
                        success = false;
                    } else {
                        brBits[0] = true;
                    }
                } else if(op[i] == 'z') {
                    if(brBits[1]) {
                        printer.printAssemblyErrorX(filename, pos, 1, inst, fileBuffer[inst->rowNum], "duplicate condition in branch");
                        success = false;
                    } else {
                        brBits[1] = true;
                    }
                } else if(op[i] == 'p') {
                    if(brBits[2]) {
                        printer.printAssemblyErrorX(filename, pos, 1, inst, fileBuffer[inst->rowNum], "duplicate condition in branch");
                        success = false;
                    } else {
                        brBits[2] = true;
                    }
                } else {
                    printer.printAssemblyErrorX(filename, pos, 1, inst, fileBuffer[inst->rowNum], "unknown branch condition '%c'", op[i]);
                    success = false;
                }
            }
        }
    } else {
        auto opcodeEntry = opcodeLUT.find(op);

        if(opcodeEntry == opcodeLUT.end()) {
            printer.printAssemblyError(filename, inst, fileBuffer[inst->rowNum], "unknown instruction \'%s\'", op.c_str());
            return false;
        } else {
            instObj = opcodeEntry->second;
        }
    }

    Token *curArg = inst->args;
    Token *firstExtraArg = nullptr;
    int count = 0;

    while(curArg != nullptr) {
        count++;

        if(firstExtraArg == nullptr && count > instObj.argCount) {
            firstExtraArg = curArg;
        }

        curArg = curArg->next;

    }

    if(count == instObj.argCount + 1) {
        printer.printAssemblyError(filename, firstExtraArg, fileBuffer[inst->rowNum], "extraneous operand \'%s\'", firstExtraArg->data.str->c_str());
    } else if(count > instObj.argCount + 1) {
        printer.printAssemblyError(filename, firstExtraArg, fileBuffer[inst->rowNum], "extraneous operands starting at \'%s\'", firstExtraArg->data.str->c_str());
    } else if(count < instObj.argCount + 1) {
        printer.printAssemblyError(filename, inst, fileBuffer[inst->rowNum], "missing operands for \'%s\'", op.c_str());
    }

    return success;
}

bool Assembler::processStatement(const std::string& filename, const Token *state)
{
    if(state->type == INST) {
        return assembleInstruction(filename, state);
    } else if(state->type == PSEUDO) {
        // return processPsuedo(state);
    } else if(state->type == LABEL) {
        // return injectSymbol(state);
    } else {
        // well this is awkward...
        return false;
    }

    return true;
}

bool Assembler::assembleProgram(const std::string& filename, const Token *program, const std::map<std::string, int> &symbolTable)
{
    std::ifstream file(filename);

    if(file.is_open()) {
        std::string line;
        const Token *curState = program;

        fileBuffer.clear();
        while(std::getline(file, line)) {
            fileBuffer.push_back(line);
        }

        file.close();

        bool success = true;
        while(curState != nullptr) {
            success &= processStatement(filename, curState);
            curState = curState->next;
        }
    }

    return true;
}
