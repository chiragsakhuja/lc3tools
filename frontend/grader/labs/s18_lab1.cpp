#include "../framework.h"

void ZeroTest(lc3::sim & sim)
{
    // Setup
    for(uint32_t i = 0x7f50; i < 0x8050; i += 1) {
        sim.setPC(0x3000);
        sim.setMem(0x4000, i);
        sim.setMem(0x4001, 0x8000);

        // Run
        sim.setRunInstLimit(1000);
        sim.run();

        // Verify
        if(i >= 0x8010) {
            VERIFY(sim.getMem(0x3200) == 0x8000);
        } else {
            VERIFY(sim.getMem(0x3200) == 0x0000);
        }
    }
}

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(void)
{
    REGISTER_TEST(Zero, ZeroTest, 70);
}
