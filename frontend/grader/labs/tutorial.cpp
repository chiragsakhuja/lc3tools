/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework2.h"

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writeMem(0x3200, 0);

    sim.run();

    grader.verify("ZeroTest", sim.readMem(0x3100) == 0, total_points);
}

void SimpleTest(lc3::sim & sim, Grader & grader, double total_points)
{
    // Initialize the values and compute their sum in C++ to compare against
    uint16_t values[] = {5, 4, 3, 2, 1, 0};
    uint64_t num_values = sizeof(values) / sizeof(uint16_t);
    uint16_t real_sum = 0;

    for(uint64_t i = 0; i < num_values; i += 1) {
        sim.writeMem(0x3200 + static_cast<uint16_t>(i), values[i]);
        real_sum += values[i];
    }

    // Run test case
    sim.setRunInstLimit(50000);
    sim.run();

    // Verify result
    grader.verify("SimpleTest", sim.readMem(0x3100) == real_sum, total_points);
}

void testBringup(lc3::sim & sim)
{
    sim.writePC(0x3000);
    sim.setRunInstLimit(50000);
}

void testTeardown(lc3::sim & sim) { (void) sim; }

void setup(Grader & grader)
{
    grader.registerTest("Zero", ZeroTest, 10, false);
    grader.registerTest("Simple", SimpleTest, 20, false);
}

void shutdown(void) {}
