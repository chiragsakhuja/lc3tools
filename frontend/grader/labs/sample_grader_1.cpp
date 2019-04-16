#include "../framework.h"

void ZeroTest(lc3::sim & sim)
{
    // Setup
    sim.setPC(0x3000);
    sim.setMem(0x3050, 0x0000);

    // Run
    sim.setRunInstLimit(1000);
    sim.run();

    // Verify
    VERIFY(sim.getMem(0x3051) == 0x0000);
}

void AllPowerTest(lc3::sim & sim)
{
    for(uint32_t i = 0; i < 16; i += 1) {
        // Setup (machine is only cleared between tests)
        sim.setPC(0x3000);
        sim.setMem(0x3050, 1 << i);

        // Run
        sim.setRunInstLimit(10000);
        sim.run();

        // Verify
        VERIFY(sim.getMem(0x3051) == 0x0001);
    }
}

void testBringup(lc3::sim & sim)
{
    (void) sim;
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(void)
{
    REGISTER_RANDOM_TEST(Zero, ZeroTest, 30);
    REGISTER_TEST(AllPowers, AllPowerTest, 60);
    REGISTER_RANDOM_TEST(AllPowers, AllPowerTest, 10);
}

void shutdown(void) {}
