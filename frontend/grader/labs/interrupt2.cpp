/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cmath>
#include <vector>
#include <string>
#include <sstream>

#include "framework.h"

static constexpr double correct_thresh = 1.0 - 0.00001;
static constexpr double close_thresh = 0.9;
static constexpr double partial_thresh = 0.2;

void verify(Grader & grader, bool success, std::string const & expected, std::string const & label, bool not_present,
    double points)
{
    if(! success) { grader.error(label, "Execution hit exception"); return; }

    bool found = grader.checkContain(grader.getOutput(), expected);
    grader.verify(label, found ^ not_present, points);
}

void ExampleTest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.setInputString("5");
    grader.setInputCharDelay(1000);
    bool success = sim.run();
    verify(grader, success, "55555", "Contains correct count", false, total_points / 2);
    verify(grader, success, "555555", "Does not contain incorrect count", true, total_points / 2);
}

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.setInputString("0");
    grader.setInputCharDelay(1000);
    bool success = sim.run();
    verify(grader, success, "0", "Does not contain zero", true, total_points);
}

void PrevASCIITest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.setInputString("@");
    grader.setInputCharDelay(1000);
    bool success = sim.run();
    verify(grader, success, "@ is not a decimal digit", "Correct behavior", false, total_points);
}

void NextASCIITest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.setInputString(":");
    grader.setInputCharDelay(1000);
    bool success = sim.run();
    verify(grader, success, ": is not a decimal digit", "Correct behavior", false, total_points);
}

void testBringup(lc3::sim & sim)
{
    sim.setRunInstLimit(10000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Grader & grader)
{
    grader.registerTest("Example", ExampleTest, 20, false);
    grader.registerTest("Example", ExampleTest, 20, true);
    grader.registerTest("Zero", ZeroTest, 20, false);
    grader.registerTest("Prev ASCII", PrevASCIITest, 20, false);
    grader.registerTest("Next ASCII", NextASCIITest, 20, false);
}

void shutdown(void) {}

