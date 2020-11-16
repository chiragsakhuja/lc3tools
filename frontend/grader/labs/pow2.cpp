/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

void verify(Grader & grader, lc3::sim & sim, bool success, uint16_t expected, double points)
{
    if(! success) { grader.error("Error", "Execution hit exception"); return; }

    grader.verify("Correct", sim.readMem(0x3051) == expected, points);
}

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    // Setup
    sim.writePC(0x3000);
    sim.setRunInstLimit(10000);
    sim.writeMem(0x3050, 0x0000);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    verify(grader, sim, success, 0, total_points);
}

void AllPowerTest(lc3::sim & sim, Grader & grader, double total_points)
{
    for(uint32_t i = 0; i < 16; i += 1) {
        // Setup (machine is only cleared between tests)
        sim.writePC(0x3000);
        sim.setRunInstLimit(10000);
        sim.writeMem(0x3050, 1 << i);

        // Run
        bool success = sim.runUntilHalt();

        // Verify
        verify(grader, sim, success, 1, total_points / 16.0);
    }
}

void SomePowerTest(lc3::sim & sim, Grader & grader, double total_points)
{
    for(uint32_t i = 0; i < 8; i += 1) {
        // Setup (machine is only cleared between tests)
        sim.writePC(0x3000);
        sim.setRunInstLimit(10000);
        sim.writeMem(0x3050, i);

        // Run
        bool success = sim.runUntilHalt();

        // Verify
        verify(grader, sim, success, ((i == 1 || i == 2 || i == 4) ? 0x0001 : 0x0000), total_points / 8.0);
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

void setup(Grader & grader)
{
    grader.registerTest("Zero", ZeroTest, 20, true);
    grader.registerTest("All Powers", AllPowerTest, 30, false);
    grader.registerTest("All Powers", AllPowerTest, 10, true);
    grader.registerTest("Some Powers", SomePowerTest, 30, false);
    grader.registerTest("Some Powers", SomePowerTest, 10, true);
}

void shutdown(void) {}
