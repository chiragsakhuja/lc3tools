#include "../framework.h"

void ZeroTest(lc3::sim & sim)
{
    sim.setPC(0x3000);
    sim.setMem(0x3200, 0);

    sim.setRunInstLimit(50000);
    sim.run();

    VERIFY(sim.getMem(0x3100) == 0);
}

void testBringup(lc3::sim & sim)
{
}

void testTeardown(lc3::sim & sim)
{
}

void setup(void)
{
    REGISTER_TEST(Zero, ZeroTest, 100);
}
