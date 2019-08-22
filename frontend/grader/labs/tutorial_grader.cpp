/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "../framework.h"

void ZeroTest(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    sim.setMem(0x3200, 0);

    sim.run();

    VERIFY(sim.getMem(0x3100) == 0);
}

void SimpleTest(lc3::sim & sim, StringInputter & inputter)
{
    (void) inputter;

    // Initialize the values and compute their sum in C++ to compare against
    uint16_t values[] = {5, 4, 3, 2, 1, 0};
    uint64_t num_values = sizeof(values) / sizeof(uint16_t);
    uint16_t real_sum = 0;

    for(uint64_t i = 0; i < num_values; i += 1) {
        sim.setMem(0x3200 + static_cast<uint16_t>(i), values[i]);
        real_sum += values[i];
    }

    // Run test case
    sim.setRunInstLimit(50000);
    sim.run();

    // Verify result
    VERIFY(sim.getMem(0x3100) == real_sum);
}

void testBringup(lc3::sim & sim)
{
    sim.setPC(0x3000);
    sim.setRunInstLimit(50000);
}

void testTeardown(lc3::sim & sim) { (void) sim; }

void setup(void)
{
    REGISTER_TEST(Zero, ZeroTest, 10);
    REGISTER_TEST(Simple, SimpleTest, 20);
}

void shutdown(void) {}
