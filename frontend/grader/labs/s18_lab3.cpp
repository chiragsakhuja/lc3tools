#include "../framework.h"

void LongTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    int address = 0x3400;
    int nxt_address = 0x3402;
    for (int i = 0; i < 511; i++){
        sim.setMem(address, nxt_address);
        sim.setMem((address + 1), 0x4500);
        address = nxt_address;
        nxt_address = address + 2;
    }
    sim.setMem(address, 0x0000);
    sim.setMem((address + 1), 0x4500);

    sim.setMemString(0x4500, "Bevo");


    // Run
    sim.setRunInstLimit(100000);
    sim.run();

    // Verify
    VERIFY_OUTPUT_NAMED("Bevo x1024", "Bevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo\nBevo");
}

void GivenTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    sim.setMem(0x3400, 0x3500);
    sim.setMem(0x3401, 0x3410);
    sim.setMemString(0x3410, "smith");

    sim.setMem(0x3500, 0x3600);
    sim.setMem(0x3501, 0x3510);
    sim.setMemString(0x3510, "spencer");

    sim.setMem(0x3600, 0x0000);
    sim.setMem(0x3601, 0x3610);
    sim.setMemString(0x3610, "yu");

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("smith\nspencer\nyu\nyu\nspencer\nsmith");
}

void NullLLTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x0000);

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("");
}

void ContiguousTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    sim.setMem(0x3400, 0x3402);
    sim.setMem(0x3401, 0x3410);
    sim.setMem(0x3402, 0x3404);
    sim.setMem(0x3403, 0x3420);
    sim.setMem(0x3404, 0x0000);
    sim.setMem(0x3405, 0x3430);

    sim.setMemString(0x3410, "smith");
    sim.setMemString(0x3420, "spencer");
    sim.setMemString(0x3430, "yu");

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("smith\nspencer\nyu\nyu\nspencer\nsmith");
}

void SameNameTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    sim.setMem(0x3400, 0x3500);
    sim.setMem(0x3401, 0x3600);

    sim.setMem(0x3500, 0x0000);
    sim.setMem(0x3501, 0x3600);

    sim.setMemString(0x3600, "smith");

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("smith\nsmith\nsmith\nsmith");
}

void EmptyNameTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    sim.setMem(0x3400, 0x3500);
    sim.setMem(0x3401, 0x3410);
    sim.setMemString(0x3410, "");

    sim.setMem(0x3500, 0x3600);
    sim.setMem(0x3501, 0x3510);
    sim.setMemString(0x3510, "");

    sim.setMem(0x3600, 0x0000);
    sim.setMem(0x3601, 0x3610);
    sim.setMemString(0x3610, "");

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("\n\n\n\n\n");
}

void OneNodeTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    sim.setMem(0x3400, 0x0000);
    sim.setMem(0x3401, 0x3410);
    sim.setMemString(0x3410, "smith");

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("smith\nsmith");
}

void NoOverwriteMemoryTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);

    sim.setMem(0x3300, 0x3400);

    sim.setMem(0x3400, 0x3500);
    sim.setMem(0x3401, 0x3410);
    sim.setMemString(0x3410, "smith");

    sim.setMem(0x3500, 0x3600);
    sim.setMem(0x3501, 0x3510);
    sim.setMemString(0x3510, "spencer");

    sim.setMem(0x3600, 0x0000);
    sim.setMem(0x3601, 0x3610);
    sim.setMemString(0x3610, "yu");

    // Run
    sim.setRunInstLimit(10000);
    sim.run();

    // Verify
    VERIFY_OUTPUT("smith\nspencer\nyu\nyu\nspencer\nsmith");
    VERIFY(sim.getMem(0x3300) == 0x3400);
    VERIFY(sim.getMem(0x3400) == 0x3500);
    VERIFY(sim.getMem(0x3401) == 0x3410);
    VERIFY(sim.getMem(0x3500) == 0x3600);
    VERIFY(sim.getMem(0x3501) == 0x3510);
    VERIFY(sim.getMem(0x3600) == 0x0000);
    VERIFY(sim.getMem(0x3601) == 0x3610);
}

void testBringup(lc3::sim & sim) { (void) sim; }

void testTeardown(lc3::sim & sim) { (void) sim; }

void setup(void)
{
    REGISTER_TEST(Given_Test, GivenTest, 5);
    REGISTER_RANDOM_TEST(Given_Test, GivenTest, 5);
    REGISTER_TEST(Null_Test, NullLLTest, 5);
    REGISTER_RANDOM_TEST(Null_Test, NullLLTest, 5);
    REGISTER_TEST(Contiguous_Test, ContiguousTest, 5);
    REGISTER_RANDOM_TEST(Contiguous_Test, ContiguousTest, 5);
    REGISTER_TEST(Same_Name_Test, SameNameTest, 5);
    REGISTER_RANDOM_TEST(Same_Name_Test, SameNameTest, 5);
    REGISTER_TEST(Empty_Name_Test, EmptyNameTest, 5);
    REGISTER_RANDOM_TEST(Empty_Name_Test, EmptyNameTest, 5);
    REGISTER_TEST(One_Node_Test, OneNodeTest, 5);
    REGISTER_RANDOM_TEST(One_Node_Test, OneNodeTest, 5);
    REGISTER_TEST(No_Overwrite_Memory_Test, NoOverwriteMemoryTest, 5);
    REGISTER_RANDOM_TEST(No_Overwrite_Memory_Test, NoOverwriteMemoryTest, 5);
    REGISTER_TEST(Long_Test, LongTest, 5);
    REGISTER_RANDOM_TEST(Long_Test, LongTest, 5);
}
