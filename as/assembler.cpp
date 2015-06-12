#include <fstream>
#include <string>
#include <map>

#include "assembler.h"
#include "tokens.h"
#include "lc3.tab.h"


std::vector<std::string> Assembler::fileBuffer;
std::map<std::string, int> Assembler::opcodeLUT;

Assembler& Assembler::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static Assembler instance;

    return instance;
}

Assembler::Assembler()
{
    opcodeLUT["add"]  = 0x1;
    opcodeLUT["and"]  = 0x5;
    opcodeLUT["br"]   = 0x0;     // unused
    opcodeLUT["jmp"]  = 0xc;
    opcodeLUT["jsr"]  = 0x4;
    opcodeLUT["jsrr"] = 0x4;
    opcodeLUT["ld"]   = 0x2;
    opcodeLUT["ldi"]  = 0xa;
    opcodeLUT["ldr"]  = 0x6;
    opcodeLUT["lea"]  = 0xe;
    opcodeLUT["not"]  = 0x9;
    opcodeLUT["ret"]  = 0xc;
    opcodeLUT["rti"]  = 0x8;
    opcodeLUT["st"]   = 0x3;
    opcodeLUT["sti"]  = 0xd;
    opcodeLUT["str"]  = 0x7;
    opcodeLUT["trap"] = 0xf;
}

bool Assembler::assembleInstruction(Token *inst)
{
    std::string *op = inst->data.str;
    int opcode = 0;     // default to br
    bool brBits[3] = {false, false, false};     // 0:n, 1:z, 2:p

    if(op->find("br") == 0) {
        // starts with br (brn, brz, etc.)

        if(*op == "br") {
            // br means always branch
            brBits[0] = true;
            brBits[1] = true;
            brBits[2] = true;
        }

        if(op->find("n") != std::string::npos) {
            brBits[0] = true;
        }

        if(op->find("z") != std::string::npos) {
            brBits[1] = true;
        }

        if(op->find("p") != std::string::npos) {
            brBits[2] = true;
        }
    } else {
        auto opcodeEntry = opcodeLUT.find(*op);
        if(opcodeEntry == opcodeLUT.end()) {
            opcode = 255;
            // return false ?
        } else {
            opcode = opcodeEntry->second;
        }
    }

    return true;
}

bool Assembler::processStatement(Token *state)
{
    if(state->type == INST) {
        return assembleInstruction(state);
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

bool Assembler::assembleProgram(const char *filename, Token *program, std::map<std::string, int> &symbolTable)
{
    std::ifstream file(filename);

    if(file.is_open()) {
        std::string line;
        Token *curState = program;

        fileBuffer.clear();
        while(std::getline(file, line)) {
            fileBuffer.push_back(line);
        }

        file.close();

        bool success = true;
        while(curState != nullptr) {
            success &= processStatement(curState);
            curState = curState->next;
        }
    }

    return true;
}
