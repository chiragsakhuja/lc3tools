/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cmath>
#include <vector>
#include <string>
#include <sstream>

#include "framework.h"

void verify(Tester & tester, bool success, std::string const & expected, std::string const & label, bool not_present,
    double points)
{
    if(! success) { tester.error(label, "Execution hit exception"); return; }

    bool found = tester.checkContain(tester.getOutput(), expected);
    tester.verify(label, found ^ not_present, points);
}

void ExampleTest(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writePC(0x0800);
    tester.setInputString("5");
    tester.setInputCharDelay(1000);
    bool success = sim.run();
    verify(tester, success, "55555", "Contains correct count", false, total_points / 2);
    verify(tester, success, "555555", "Does not contain incorrect count", true, total_points / 2);
}

void ZeroTest(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writePC(0x0800);
    tester.setInputString("0");
    tester.setInputCharDelay(1000);
    bool success = sim.run();
    verify(tester, success, "0", "Does not contain zero", true, total_points);
}

void PrevASCIITest(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writePC(0x0800);
    tester.setInputString("@");
    tester.setInputCharDelay(1000);
    bool success = sim.run();
    verify(tester, success, "@ is not a decimal digit", "Correct behavior", false, total_points);
}

void NextASCIITest(lc3::sim & sim, Tester & tester, double total_points)
{
    sim.writePC(0x0800);
    tester.setInputString(":");
    tester.setInputCharDelay(1000);
    bool success = sim.run();
    verify(tester, success, ": is not a decimal digit", "Correct behavior", false, total_points);
}

void testBringup(lc3::sim & sim)
{
    sim.setRunInstLimit(10000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Tester & tester)
{
    tester.registerTest("Example", ExampleTest, 20, false);
    tester.registerTest("Example", ExampleTest, 20, true);
    tester.registerTest("Zero", ZeroTest, 20, false);
    tester.registerTest("Prev ASCII", PrevASCIITest, 20, false);
    tester.registerTest("Next ASCII", NextASCIITest, 20, false);
}

void shutdown(void) {}

