#include "../framework.h"

void Helper(lc3::sim & sim, uint32_t a, uint32_t b)
{
    sim.setPC(0x3000);
    sim.setMem(0x4000, a);
    sim.setMem(0x4001, b);
}

void SimpleTest(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x0000, 0x0000);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x0000);
}

void ExampleTest1(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x8000, 0x8000);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x8000);
}

void ExampleTest2(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x1234, 0x5678);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x0000);
}

void PositiveTest(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x7FFF, 0x7FFF);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x8000);
}

void NegativeTest(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x8000, 0xFFFF);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x8000);
}

void All_Ones_Test(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0xFFFF, 0xFFFF);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x0000);
}

void Add_One_Test(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x7FFF, 0x0001);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x8000);
}

void Sub_One_Test(lc3::sim & sim)
{
    // Setup
    Helper(sim, 0x7FFF, 0xFFFF);

    // Run
    sim.setRunInstLimit(2000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3200) == 0x0000);
}

void testBringup(lc3::sim & sim) { (void) sim; }

void testTeardown(lc3::sim & sim) { (void) sim; }

void setup(void)
{
    REGISTER_TEST(Zero_Case, SimpleTest, 5);
    REGISTER_RANDOM_TEST(Zero_Case_Random, SimpleTest, 5);
    REGISTER_TEST(Example1, ExampleTest1, 10);
    REGISTER_RANDOM_TEST(Example1_Random, ExampleTest1, 10);
    REGISTER_TEST(Example2, ExampleTest2, 10);
    REGISTER_RANDOM_TEST(Example2_Random, ExampleTest2, 10);
    REGISTER_TEST(Positive_Overflow,PositiveTest, 5);
    REGISTER_RANDOM_TEST(Positive_Overflow__Random, PositiveTest, 5);
    REGISTER_TEST(Negative_Overflow,NegativeTest, 5);
    REGISTER_RANDOM_TEST(Negative_Overflow_Random, NegativeTest, 5);
    REGISTER_TEST(All_Ones,All_Ones_Test, 5);
    REGISTER_RANDOM_TEST(All_Ones_Random, All_Ones_Test, 5);
    REGISTER_TEST(Add_One,Add_One_Test, 5);
    REGISTER_RANDOM_TEST(Add_One_Random, Add_One_Test, 5);
    REGISTER_TEST(Sub_One,Sub_One_Test, 5);
    REGISTER_RANDOM_TEST(Sub_One_Random, Sub_One_Test, 5);
}
