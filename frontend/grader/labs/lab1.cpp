#include "../framework.h"

void ZeroTest(void)
{
    // Setup
    simSetMemVal(0x3050, 0x0000);

    // Run
    simSetRunInstLimit(10000);
    simRun();

    // Verify
    VERIFY(simGetMemVal(0x3051) == 0x0000);
}

void AllPowerTest(void)
{
    for(uint32_t i = 0; i < 16; i += 1) {
        // Setup (machine is only cleared between tests)
        simSetPC(0x3000);
        simSetMemVal(0x3050, 1 << i);

        // Run
        simSetRunInstLimit(10000);
        simRun();

        // Verify
        VERIFY(simGetMemVal(0x3051) == 0x0001);
    }
}

void setup(void)
{
    REGISTER_RANDOM_TEST(Zero, ZeroTest, 30);
    REGISTER_TEST(AllPowers, AllPowerTest, 60);
    REGISTER_RANDOM_TEST(AllPowers, AllPowerTest, 10);
}
