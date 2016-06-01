#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstdarg>
#include <string>
#include <cstdint>
#include <fstream>

#include "tokens.h"
#include "parser.hpp"
#include "paths.h"
#include "instruction_encoder.h"

#include "../../src/utils/printer.h"
#include "../../src/utils/console_printer.h"
#include "../../src/as/logger.h"

#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

#define _ASSEMBLER_TEST
#include "assembler.h"
#undef _ASSEMBLER_TEST

std::map<std::string, int> globalSymbols;
std::map<std::string, int> localSymbols;

void destroyInstruction(Token *inst)
{
    Token *oper = inst->opers;

    while(oper != nullptr) {
        Token *temp = oper->next;
        delete oper;
        oper = temp;
    }

    delete inst;
}

void loadTest(const std::string& testName)
{
}

class AssemblerSimple : public ::testing::Test
{
protected:
    AssemblerSimple()
    {
        std::string testPath(globalTestPath);
        std::ifstream file(testPath + "/assembler/assembler_simple.txt");

        if(file.is_open()) {
            std::string word;

            while(file >> word) {
                if(word == "global") {
                    std::string address;

                    file >> word;
                    file >> address;

                    globalSymbols[word] = std::stoi(address, 0, 16);
                }
            }

            file.close();
        } else {
            std::cout << "Could not open test file";
        }
    }

    virtual ~AssemblerSimple() { }

    virtual void SetUp()
    {
        localSymbols.clear();
    }

    virtual void TearDown() { }

    std::list<Token *> readTest(const std::string& testName)
    {
        std::string testPath(globalTestPath);
        std::ifstream file(testPath + "/assembler/assembler_simple.txt");
        std::list<Token *> programs;

        if(file.is_open()) {
            std::string word;

            while(file >> word) {
                if(word == "test") {
                    file >> word;

                    if(word != testName) {
                        while(word != "endtest") { file >> word; }
                    } else {
                        break;
                    }
                }
            }

            while(word != "endtest") {
                file >> word;

                if(word == "local") {
                    std::string address;

                    file >> word;
                    file >> address;

                    localSymbols[word] = std::stoi(address, 0, 16);
                } else if(word == "orig") {
                    file >> word;

                    int pc = std::stoi(word, 0, 16);

                    Token *program = nullptr, *prevInst = nullptr;
                    while(word != "endorig") {
                        // get encoding
                        file >> word;
                        if(word == "endorig") {
                            break;
                        }
                        int encoding = std::stoi(word, 0, 16);

                        // get instruction name
                        file >> word;

                        Token *newInst = new Token(word);
                        newInst->type = INST;
                        newInst->pc = pc;
                        newInst->encoding = encoding;

                        // get number of operands
                        file >> word;
                        int numOpers = std::stoi(word);
                        newInst->num_operands = numOpers;

                        // iterate over operands
                        Token *prevOper = nullptr;
                        for(int i = 0; i < numOpers; i++) {
                            Token *newOper = nullptr;

                            file >> word;

                            if(word == "reg") {
                                file >> word;

                                newOper = new Token(word);
                                newOper->type = OPER_TYPE_REG;
                            } else if(word == "imm") {
                                file >> word;

                                newOper = new Token(std::stoi(word));
                                newOper->type = OPER_TYPE_IMM;
                            }

                            // append operand to instruction
                            if(prevOper == nullptr) {
                                newInst->opers = newOper;
                            } else {
                                prevOper->next = newOper;
                            }

                            prevOper = newOper;
                        }

                        // append instruction to program
                        if(program == nullptr) {
                            program = newInst;
                        } else {
                            prevInst->next = newInst;
                        }

                        prevInst = newInst;
                    }

                    programs.push_back(program);
                }
            }

            file.close();
        } else {
            std::cout << "Could not open test file";
        }

        return programs;
    }
};

TEST_F(AssemblerSimple, SingleDataProcessingInstruction)
{
    std::list<Token *> programs = readTest("single_data_processing_instruction");

    Assembler as;
    utils::Printer const * printer = new utils::ConsolePrinter();
    AssemblerLogger logger(*printer);
    InstructionEncoder encoder(false, *printer);
    std::map<std::string, int> symbolTable;
    uint32_t encodedInstructon;
    bool status = false;

    for(auto it = programs.begin(); it != programs.end(); it++) {
        EXPECT_TRUE(status = as.processInstruction(false, logger, encoder, "", *it, symbolTable, encodedInstructon));
        if(status) {
            EXPECT_EQ((*it)->encoding, encodedInstructon);
        }
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
