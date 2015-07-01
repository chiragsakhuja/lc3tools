#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>

#include "tokens.h"
#include "utils/printer.h"
#include "assembler_printer.h"
#include "thirdparty/jsonxx/jsonxx.h"
#include "instruction_encoder.h"

#include "paths.h"

std::map<std::string, std::list<Instruction *> > InstructionEncoder::insts;
int InstructionEncoder::regWidth = 32;
std::vector<std::string> InstructionEncoder::regs;

Operand::Operand() : Operand(OPER_TYPE_UNKNOWN, 0, 0) { }

Operand::Operand(int type, int lo, int hi)
{
    this->type = type;
    this->lo = lo;
    this->hi = hi;
}

bool Operand::compareTypes(int otherType)
{
    if(otherType == OPER_TYPE_REG && type == OPER_TYPE_REG) {
        return true;
    } else if(   otherType == OPER_TYPE_LABEL
              && (   type == OPER_TYPE_IMMS
                  || type == OPER_TYPE_IMMU
                  || type == OPER_TYPE_PCOFFS
                  || type == OPER_TYPE_PCOFFU
                 )
             )
    {
        return true;
    }

    return false;
}

Instruction::Instruction(int width, bool setcc, const std::string& label)
{
    bitTypes = new int[width];
    std::fill_n(bitTypes, width, OPER_TYPE_UNKNOWN);

    this->setcc = setcc;
    this->label = label;
}

Instruction::~Instruction()
{
    delete[] bitTypes;
    bitTypes = nullptr;

    for(auto it = operTypes.begin(); it != operTypes.end(); it++) {
        delete *it;
    }
}

InstructionEncoder& InstructionEncoder::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static InstructionEncoder instance;

    return instance;
}

InstructionEncoder::InstructionEncoder()
{
    Printer& printer = Printer::getInstance();
    std::string resPath(globalResPath);


    std::ifstream file(resPath + "/encodings.json");

    if(file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();

        jsonxx::Array list;
        list.parse(buffer.str());

        for(size_t i = 0; i < list.size(); i++) {
            if(!list.has<jsonxx::Object>(i)) {
                printer.printfMessage(Printer::ERROR, "unknown encoding");
                continue;
            }

            jsonxx::Object state = list.get<jsonxx::Object>(i);

            if(!state.has<jsonxx::String>("type") || !state.has<jsonxx::Object>("data")) {
                printer.printfMessage(Printer::ERROR, "incorrect encoding");
                continue;
            }

            jsonxx::String type = state.get<jsonxx::String>("type");
            jsonxx::Object data = state.get<jsonxx::Object>("data");

            // begin the giant parser
            if(type == "CONF") {    // start config
                if(!data.has<jsonxx::Number>("width")) {
                    printer.printfMessage(Printer::ERROR, "unspecified width in encoding");
                    continue;
                }

                jsonxx::Number width = data.get<jsonxx::Number>("width");
                regWidth = width;

                printer.printfMessage(Printer::DEBUG, "configuring %d-bit registers", regWidth);
            } else if(type == "REGS") {     // end config, start regs
                if(!data.has<jsonxx::Array>("reglist")) {
                    printer.printfMessage(Printer::ERROR, "unspecified reglist in encoding");
                    continue;
                }

                jsonxx::Array reglist = data.get<jsonxx::Array>("reglist");
                std::stringstream output;
                output << "configuring registers ";

                for(size_t j = 0; j < reglist.size(); j++) {
                    if(!reglist.has<jsonxx::String>(j)) {
                        printer.printfMessage(Printer::ERROR, "unknown type in reglist");
                        continue;
                    }

                    jsonxx::String reg = reglist.get<jsonxx::String>(j);

                    regs.push_back(reglist.get<jsonxx::String>(j));
                    output << reg << " ";
                }


                printer.printfMessage(Printer::DEBUG, output.str().c_str());
            } else if(type == "INST") {     // end regs, start inst
                if(!data.has<jsonxx::String>("label") || !data.has<jsonxx::Boolean>("setcc") || !data.has<jsonxx::Array>("enc")) {
                    printer.printfMessage(Printer::ERROR, "unspecified fields in instruction encoding");
                    continue;
                }

                jsonxx::String label = data.get<jsonxx::String>("label");
                jsonxx::Boolean setcc = data.get<jsonxx::Boolean>("setcc");
                jsonxx::Array enc = data.get<jsonxx::Array>("enc");

                std::list<Instruction *> *sameOpInsts = nullptr;
                if(regWidth == 0) {
                    printer.printfMessage(Printer::WARNING, "unconfigured register width, assuming 32-bit");
                }

                Instruction *newInst = new Instruction(regWidth, setcc, label);

                for(size_t j = 0; j < enc.size(); j++) {
                    if(!enc.has<jsonxx::Object>(j)) {
                        printer.printfMessage(Printer::ERROR, "unknown type in %s instruction encoding", label.c_str());
                        continue;
                    }

                    jsonxx::Object inst = enc.get<jsonxx::Object>(j);

                    if(!inst.has<jsonxx::Number>("hi") || !inst.has<jsonxx::Number>("lo") || !inst.has<jsonxx::String>("type") || !inst.has<jsonxx::Object>("data")) {
                        printer.printfMessage(Printer::ERROR, "unspecified fields in %s instruction encoding", label.c_str());
                        continue;
                    }

                    jsonxx::Number hi = inst.get<jsonxx::Number>("hi");
                    jsonxx::Number lo = inst.get<jsonxx::Number>("lo");
                    jsonxx::String instType = inst.get<jsonxx::String>("type");
                    jsonxx::Object instData = inst.get<jsonxx::Object>("data");

                    if(instType == "OPCODE" || instType == "FIXED") {
                        if(!instData.has<jsonxx::String>("value")) {
                            printer.printfMessage(Printer::ERROR, "unspecified value in fixed encoding for %s instruction", label.c_str());
                            continue;
                        }

                        jsonxx::String value = instData.get<jsonxx::String>("value");

                        if(instType == "OPCODE") {
                            sameOpInsts = &insts[label];
                        }

                        for(int k = (int) lo; k <= (int) hi; k++) {
                            newInst->bitTypes[k] = value[hi - k] - '0';
                        }
                    } else if(instType == "REG" || instType == "IMMS" || instType == "IMMU" || instType == "PCOFFS" || instType == "PCOFFU") {
                        if(!instData.has<jsonxx::Number>("pos")) {
                            printer.printfMessage(Printer::ERROR, "unspecified pos in dynamic encoding for %s instruction", label.c_str());
                            continue;
                        }

                        jsonxx::Number pos = instData.get<jsonxx::Number>("pos");
                        int type = 0;

                        if(instType == "REG") {
                            type = OPER_TYPE_REG;
                        } else if(instType == "IMMS") {
                            type = OPER_TYPE_IMMS;
                        } else if(instType == "IMMU") {
                            type = OPER_TYPE_IMMU;
                        } else if(instType == "PCOFFS") {
                            type = OPER_TYPE_PCOFFS;
                        } else if(instType == "PCOFFU") {
                            type = OPER_TYPE_PCOFFU;
                        }

                        newInst->operTypes.insert(newInst->operTypes.begin() + ((int) pos), new Operand(type, lo, hi));
                        std::fill_n(newInst->bitTypes + (int) lo, (int) (hi - lo + 1), type);
                    } else {
                        printer.printfMessage(Printer::ERROR, "unknown encoding type %s", type.c_str());
                        continue;
                    }
                }

                sameOpInsts->push_back(newInst);
            }
        }

        for(auto it = insts.begin(); it != insts.end(); it++) {
            std::list<Instruction *>& encs = it->second;

            for(auto it2 = encs.begin(); it2 != encs.end(); it2++) {
                std::stringstream output;
                Instruction *inst = *it2;

                output << inst->label << ": { ";

                for(int i = regWidth - 1; i >= 0; i--) {
                    output << inst->bitTypes[i];
                }

                output << " }";
                printer.printfMessage(Printer::DEBUG, "configuring instruction %s", output.str().c_str());
            }
        }

        file.close();
    } else {
        printer.printfMessage(Printer::ERROR, "could not open encodings.json");
    }
}

InstructionEncoder::~InstructionEncoder()
{
    for(auto map_it = insts.begin(); map_it != insts.end(); map_it++) {
        std::list<Instruction *>& encs = map_it->second;

        for(auto list_it = encs.begin(); list_it != encs.end(); list_it++) {
            delete *list_it;
        }
    }

    insts.clear();
}

// precondition: the instruction is of type pattern and is valid (no error checking)
bool InstructionEncoder::encodeInstruction(const Instruction *pattern, const Token *inst, uint32_t& encodedInstruction)
{
    bool success = true;
    const AssemblerPrinter& printer = AssemblerPrinter::getInstance();

    int *bits = new int[regWidth];
    std::fill_n(bits, regWidth, OPER_TYPE_UNKNOWN);

    const Token *curOper = inst->opers;

    for(auto it = pattern->operTypes.begin(); it != pattern->operTypes.end(); it++) {
        if((*it)->type == OPER_TYPE_REG) {
            unsigned int regNum = 0;

            for(regNum = 0; regNum < regs.size(); regNum++) {
                if(regs[regNum] == *curOper->data.str) {
                    break;
                }
            }

            if(regNum > (1U << ((*it)->hi - (*it)->lo))) {
                // TODO: print warning
                success = false;
                continue;
            }

            for(int i = (*it)->lo; i <= (*it)->hi; i++) {
                bits[i] = regNum & 1;
                regNum >>= 1;
            }
        }

        curOper = curOper->next;
    }

    for(int i = 0; i < regWidth; i++) {
        if(pattern->bitTypes[i] == 1 || pattern->bitTypes[i] == 0) {
            bits[i] = pattern->bitTypes[i];
        }
    }

    // sanity check
    std::cout << "Encoded: ";
    for(int i = regWidth - 1; i >= 0; i--) {
        if(bits[i] == OPER_TYPE_UNKNOWN) {
            success = false;
            break;
        } else {
            std::cout << bits[i];
        }
    }
    std::cout << std::endl;

    delete[] bits;

    return success;
}
