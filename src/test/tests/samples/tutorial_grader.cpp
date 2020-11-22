/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

void ZeroTest(lc3::sim & sim, Tester & tester, double total_points)
{
    // Initialize values
    sim.writeMem(0x3200, 0);

    // Run test case
    sim.run();

    // Verify result
    tester.verify("Is Zero?", sim.readMem(0x3100) == 0, total_points);
}

void SimpleTest(lc3::sim & sim, Tester & tester, double total_points)
{
    // Initialize values and compute their sum in C++ to compare against
    uint16_t values[] = {5, 4, 3, 2, 1, 0};
    uint64_t num_values = sizeof(values) / sizeof(uint16_t);
    uint16_t real_sum = 0;
    for(uint64_t i = 0; i < num_values; i += 1) {
        sim.writeMem(0x3200 + static_cast<uint16_t>(i), values[i]);
        real_sum += values[i];
    }

    // Run test case
    sim.run();

    // Verify result
    tester.verify("Is Correct?", sim.readMem(0x3100) == real_sum, total_points);
}

void testBringup(lc3::sim & sim)
{
    // Set up initial PC
    sim.writePC(0x3000);

    // Limit the number of instructions simulated
    sim.setRunInstLimit(50000);
} 

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester)
{
    // Register test cases
    tester.registerTest("Zero Test", ZeroTest, 10, false);
    tester.registerTest("Simple Test", SimpleTest, 20, false);
}

void shutdown(void) { }
