/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#define API_VER 2
#include "framework.h"

uint32_t sub_count;

void verify(Tester & tester, lc3::sim & sim, bool success, uint16_t expected_val, uint64_t expected_sub_count,
    double points)
{
    if(! success) { tester.error("Error", "Execution hit exception"); return; }

    tester.verify("Correct functionality", sim.readMem(0x4000) == expected_val, points * 0.8);
    tester.verify("Correct time complexity", sub_count == expected_sub_count, points * 0.2);
}

void LinearTest(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writeMem(0x4001, -100);
    sim.writeMem(0x4002, 100);
    sim.writeMem(0x4003, 1);
    sim.writeMem(0x4004, 2);
    sim.writeMem(0x4005, 6);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    verify(tester, sim, success, static_cast<uint16_t>(-3), 24, total_points);
}

void QuadraticTest1(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writeMem(0x4001, 2);
    sim.writeMem(0x4002, 100);
    sim.writeMem(0x4003, 2);
    sim.writeMem(0x4004, 1);
    sim.writeMem(0x4005, -4);
    sim.writeMem(0x4006, 0);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    verify(tester, sim, success, static_cast<uint16_t>(4), 21, total_points);
}

void QuadraticTest2(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writeMem(0x4001, -100);
    sim.writeMem(0x4002, 2);
    sim.writeMem(0x4003, 2);
    sim.writeMem(0x4004, 1);
    sim.writeMem(0x4005, -4);
    sim.writeMem(0x4006, 0);

    // Run
    bool success = sim.runUntilHalt();

    // Verify
    verify(tester, sim, success, static_cast<uint16_t>(0), 18, total_points);
}

void subEnterCallback(lc3::core::CallbackType type, lc3::core::MachineState & state)
{
    (void) type;

    if(state.readPC() == 0x5000) {
        sub_count += 1;
    }
}

void testBringup(lc3::sim & sim)
{
    sim.registerCallback(lc3::core::CallbackType::SUB_ENTER, subEnterCallback);
    sub_count = 0;
    sim.writePC(0x3000);
    sim.setRunInstLimit(10000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Tester & tester)
{
    tester.registerTest("Linear", LinearTest, 30, false);
    tester.registerTest("Linear", LinearTest, 10, true);
    tester.registerTest("Quadratic 1", QuadraticTest1, 20, false);
    tester.registerTest("Quadratic 1", QuadraticTest1, 10, true);
    tester.registerTest("Quadratic 2", QuadraticTest2, 20, false);
    tester.registerTest("Quadratic 2", QuadraticTest2, 10, true);
}

void shutdown(void) {}
