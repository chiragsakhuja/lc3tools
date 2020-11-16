/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

void verify(Grader & grader, bool success, std::string const & expected, double points)
{
    if(! success) { grader.error("Error", "Execution hit exception"); return; }

    bool found_match = grader.checkContain(grader.getOutput(), expected);
    grader.verify("Correct", found_match, points);
}

void UpperCaseTest(lc3::sim & sim, Grader & grader, double total_points)
{
    grader.setInputString("a");
    grader.setInputCharDelay(50);
    bool success = sim.run();
    verify(grader, success, "a is not a capital letter of the English alphabet", total_points);
}

void LowerCaseTest(lc3::sim & sim, Grader & grader, double total_points)
{
    grader.setInputString("A");
    grader.setInputCharDelay(50);
    bool success = sim.run();
    verify(grader, success, "The lower case of A is a", total_points);
}

void testBringup(lc3::sim & sim)
{
    sim.writePC(0x3000);
    sim.setRunInstLimit(10000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Grader & grader)
{
    grader.registerTest("Upper Case", UpperCaseTest, 40, false);
    grader.registerTest("Upper Case", UpperCaseTest, 10, true);
    grader.registerTest("Lower Case", LowerCaseTest, 40, false);
    grader.registerTest("Lower Case", LowerCaseTest, 10, true);
}

void shutdown(void) {}
