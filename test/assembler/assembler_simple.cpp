#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstdarg>
#include <string>
#include <cstdint>

#include "tokens.h"
#include "instruction_encoder.h"

#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

#define _ASSEMBLER_TEST
#include "assembler.h"
#undef _ASSEMBLER_TEST

void PrintTo(const Token* inst, std::ostream* os)
{
    *os << inst->str;
}

Token * buildInstruction(const std::string& label, int numOpers, ...)
{
    Token *ret = new Token(label);
    Token *prevOper = nullptr;
    ret->numOperands = numOpers;

    va_list args;
    va_start(args, numOpers);

    for(int i = 0; i < numOpers; i++) {
        int type = va_arg(args, int);
        Token *temp = nullptr;

        if(type == OPER_TYPE_REG) {
            const char *reg = va_arg(args, const char *);
            temp = new Token(reg);
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

TEST(AssemblerSimple, SingleInstruction)
{
    const Assembler& as = Assembler::getInstance();
    std::map<std::string, int> symbolTable;
    uint32_t encodedInstructon;

    Token *add1 = buildInstruction("add", 3, OPER_TYPE_REG, "r0", OPER_TYPE_REG, "r1", OPER_TYPE_REG, "r2");
    EXPECT_TRUE(as.processInstruction(false, "", add1, symbolTable, encodedInstructon));
    destroyInstruction(add1);

    EXPECT_EQ(0x1042, encodedInstructon);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
