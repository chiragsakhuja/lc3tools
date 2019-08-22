/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "../framework.h"

uint32_t sub_count;

void LinearTest(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    sim.setMem(0x4001, -100);
    sim.setMem(0x4002, 100);
    sim.setMem(0x4003, 1);
    sim.setMem(0x4004, 2);
    sim.setMem(0x4005, 6);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    VERIFY(sim.getMem(0x4000) == static_cast<uint16_t>(-3));
    VERIFY(sub_count == 24);
    VERIFY(success && ! sim.didExceedInstLimit());
}

void QuadraticTest1(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    sim.setMem(0x4001, 2);
    sim.setMem(0x4002, 100);
    sim.setMem(0x4003, 2);
    sim.setMem(0x4004, 1);
    sim.setMem(0x4005, -4);
    sim.setMem(0x4006, 0);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    VERIFY(sim.getMem(0x4000) == 4);
    VERIFY(sub_count == 21);
    VERIFY(success && ! sim.didExceedInstLimit());
}

void QuadraticTest2(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    sim.setMem(0x4001, -100);
    sim.setMem(0x4002, 2);
    sim.setMem(0x4003, 2);
    sim.setMem(0x4004, 1);
    sim.setMem(0x4005, -4);
    sim.setMem(0x4006, 0);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    VERIFY(sim.getMem(0x4000) == 0);
    VERIFY(sub_count == 18);
    VERIFY(success && ! sim.didExceedInstLimit());
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
    sub_count = 0;
    sim.setPC(0x3000);
    sim.setRunInstLimit(10000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(void)
{
    REGISTER_TEST(Linear, LinearTest, 30);
    REGISTER_RANDOM_TEST(Linear, LinearTest, 10);
    REGISTER_TEST(Quadratic1, QuadraticTest1, 20);
    REGISTER_RANDOM_TEST(Quadratic1, QuadraticTest1, 10);
    REGISTER_TEST(Quadratic2, QuadraticTest2, 20);
    REGISTER_RANDOM_TEST(Quadratic2, QuadraticTest2, 10);
}

void shutdown(void) {}
