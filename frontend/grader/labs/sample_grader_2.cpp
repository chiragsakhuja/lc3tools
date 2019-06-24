#include "../framework.h"

uint32_t sub_count = 0;

void BasicTest(lc3::sim & sim, StringInputter & inputter)
{
    // Setup
    sub_count = 0;

    // Run
    sim.setRunInstLimit(10000);
    sim.runUntilHalt();

    // Verify
    VERIFY(sim.getReg(1) == 0x0000);
    VERIFY(sub_count == 1);
}

void subEnterCallback(lc3::core::MachineState & state)
{
    if(state.pc == 0x5000) {
        sub_count += 1;
    }
}

void testBringup(lc3::sim & sim)
{
    sim.registerSubEnterCallback(subEnterCallback);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(void)
{
    REGISTER_RANDOM_TEST(Basic, BasicTest, 100);
}

void shutdown(void) {}
