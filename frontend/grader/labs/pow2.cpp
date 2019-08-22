/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "../framework.h"

void ZeroTest(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    // Setup
    sim.setPC(0x3000);
    sim.setRunInstLimit(10000);
    sim.setMem(0x3050, 0x0000);

    // Run
    bool status = sim.runUntilHalt();

    // Verify
    VERIFY(sim.getMem(0x3051) == 0x0000);
    VERIFY(status && ! sim.didExceedInstLimit());
}

void AllPowerTest(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    for(uint32_t i = 0; i < 16; i += 1) {
        // Setup (machine is only cleared between tests)
        sim.setPC(0x3000);
        sim.setRunInstLimit(10000);
        sim.setMem(0x3050, 1 << i);

        // Run
        bool status = sim.runUntilHalt();

        // Verify
        VERIFY(sim.getMem(0x3051) == 0x0001);
        VERIFY(status && ! sim.didExceedInstLimit());
        break;
    }
}

void SomePowerTest(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    for(uint32_t i = 0; i < 8; i += 1) {
        // Setup (machine is only cleared between tests)
        sim.setPC(0x3000);
        sim.setRunInstLimit(10000);
        sim.setMem(0x3050, i);

        // Run
        bool status = sim.runUntilHalt();

        // Verify
        VERIFY(sim.getMem(0x3051) == ((i == 1 || i == 2 || i == 4) ? 0x0001 : 0x0000));
        VERIFY(status && ! sim.didExceedInstLimit());
        break;
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
    REGISTER_RANDOM_TEST(Zero, ZeroTest, 20);
    REGISTER_TEST(AllPowers, AllPowerTest, 30);
    REGISTER_RANDOM_TEST(AllPowers, AllPowerTest, 10);
    REGISTER_TEST(SomePowers, SomePowerTest, 30);
    REGISTER_RANDOM_TEST(SomePowers, SomePowerTest, 10);
}

void shutdown(void) {}
