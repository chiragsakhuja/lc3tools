#include "framework.h"

void zero_test(void)
{
    // Setup
    simSetMemVal(0x3050, 0x0000);

    // Run
    simRunFor(10000);

    // Verify
    VERIFY(simGetMemVal(0x3051) == 0x0000);
}

void all_power_test(void)
{
    for(uint32_t i = 0; i < 16; i += 1) {
        // Setup
        simSetMemVal(0x3050, 1 << i);

        // Run
        simRunFor(10000);

        // Verify
        VERIFY(simGetMemVal(0x3051) == 0x0001);
    }
}

void setup(void) {
    REGISTER_TEST(Zero, zero_test, 20);
    REGISTER_RANDOM_TEST(Zero, zero_test, 10);
    REGISTER_TEST(AllPowers, all_power_test, 60);
    REGISTER_RANDOM_TEST(AllPowers, all_power_test, 10);
}
