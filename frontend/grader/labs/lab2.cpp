#include "../framework.h"

uint32_t sub_count = 0;

void BasicTest(void)
{
    // Setup
    sub_count = 0;

    // Run
    simSetRunInstLimit(10000);
    simRunUntilBeforeHalt();

    // Verify
    VERIFY(simGetReg(1) == 0x0000);
    VERIFY(sub_count == 1);
}

void subEnterCallback(core::MachineState & state)
{
    if(state.pc == 0x5000) {
        sub_count += 1;
    }
}

void setup(void)
{
    simRegisterSubEnterCallback(subEnterCallback);
    REGISTER_RANDOM_TEST(Basic, BasicTest, 100);
}
