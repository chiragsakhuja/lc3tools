#include <string>
#include <vector>
#include <map>

#include "tokens.h"
#include "assembler.h"
#include "gtest/gtest.h"

namespace
{
    class SimpleAssemblerTest : public ::testing::Test
    {
    };
};

TEST(SimpleAssemblerTest, SingleInstruction)
{
    const Assembler& as = Assembler::getInstance();
    EXPECT_EQ(0, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
