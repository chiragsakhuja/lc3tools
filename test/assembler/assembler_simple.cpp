#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstdarg>
#include <string>
#include <cstdint>
#include <fstream>

#include "tokens.h"
#include "paths.h"
#include "instruction_encoder.h"

#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

#define _ASSEMBLER_TEST
#include "assembler.h"
#undef _ASSEMBLER_TEST

std::map<std::string, int> globalSymbols;
std::map<std::string, int> localSymbols;

Token * buildInstruction(const std::string& label, int numOpers, ...)
{
    Token *ret = new Token(new std::string(label));
    Token *prevOper = nullptr;
    ret->numOperands = numOpers;

    va_list args;
    va_start(args, numOpers);

    for(int i = 0; i < numOpers; i++) {
        int type = va_arg(args, int);
        Token *temp = nullptr;

        if(type == OPER_TYPE_REG) {
            const char *reg = va_arg(args, const char *);
            temp = new Token(new std::string(reg));
            temp->type = OPER_TYPE_REG;
        }

        if(prevOper == nullptr) {
            ret->opers = temp;
        } else {
            prevOper->next = temp;
        }

        prevOper = temp;
    }

    va_end(args);

    return ret;
}

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
        std::ifstream file = std::ifstream(testPath + "/assembler/assembler_simple.txt");

        if(file.is_open()) {
            std::string word;

            while(file >> word) {
                if(word == "global") {
                    std::string label;
                    std::string address;

                    file >> label;
                    file >> address;

                    globalSymbols[label] = std::stoi(address);
                }
            }

            file.close();
        } else {
            std::cout << "Could not open test file";
        }
    }

    virtual ~AssemblerSimple()
    {
    }

    virtual void SetUp()
    {
        localSymbols.clear();
    }

    virtual void TearDown()
    {
    }

    void readTest(const std::string& testName)
    {
        std::string testPath(globalTestPath);
        std::ifstream file = std::ifstream(testPath + "/assembler/assembler_simple.txt");

        if(file.is_open()) {
            std::string word;

            while(file >> word) {
                if(word == "test") {
                    file >> word;

                    if(word != testName) {
                        while(word != "endtest") { file >> word; }
                    } else {
                        goto readTest;
                    }
                }
            }

readTest:
            while(word != "endtest") {
                file >> word;

                if(word == "local") {
                    std::string label;
                    std::string address;

                    file >> label;
                    file >> address;

                    localSymbols[label] = std::stoi(address);
                } else if(word == "orig") {
                    file >> word;

                    int pc = std::stoi(word);

                    //std::cout << "\ntestcase\n";
                    while(word != "endorig") {
                        file >> word;
                        //std::cout << " " << word;
                    }
                }
            }

            file.close();
        } else {
            std::cout << "Could not open test file";
        }
    }
};

TEST_F(AssemblerSimple, SingleDataProcessingInstruction)
{
    readTest("single_data_processing_instruction");

    const Assembler& as = Assembler::getInstance();
    std::map<std::string, int> symbolTable;
    uint32_t encodedInstructon;
    bool status = false;
    Token *inst = nullptr;

    inst = buildInstruction("add", 3, OPER_TYPE_REG, "r0", OPER_TYPE_REG, "r1", OPER_TYPE_REG, "r2");
    EXPECT_TRUE(status = as.processInstruction(false, "", inst, symbolTable, encodedInstructon));
    if(status) {
        EXPECT_EQ(0x1042, encodedInstructon);
    }
    destroyInstruction(inst);

    inst = buildInstruction("and", 3, OPER_TYPE_REG, "r0", OPER_TYPE_REG, "r1", OPER_TYPE_REG, "r2");
    EXPECT_TRUE(status = as.processInstruction(false, "", inst, symbolTable, encodedInstructon));
    if(status) {
        EXPECT_EQ(0x5042, encodedInstructon);
    }
    destroyInstruction(inst);

    inst = buildInstruction("not", 2, OPER_TYPE_REG, "r0", OPER_TYPE_REG, "r1");
    EXPECT_TRUE(status = as.processInstruction(false, "", inst, symbolTable, encodedInstructon));
    if(status) {
        EXPECT_EQ(0x907f, encodedInstructon);
    }
    destroyInstruction(inst);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
