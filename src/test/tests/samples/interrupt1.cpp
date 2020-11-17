/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

void verify(Tester & tester, bool success, std::string const & expected, double points)
{
    if(! success) { tester.error("Error", "Execution hit exception"); return; }

    bool found_match = tester.checkContain(tester.getOutput(), expected);
    tester.verify("Correct", found_match, points);
}

void UpperCaseTest(lc3::sim & sim, Tester & tester, double total_points)
{
    tester.setInputString("a");
    tester.setInputCharDelay(50);
    bool success = sim.run();
    verify(tester, success, "a is not a capital letter of the English alphabet", total_points);
}

void LowerCaseTest(lc3::sim & sim, Tester & tester, double total_points)
{
    tester.setInputString("A");
    tester.setInputCharDelay(50);
    bool success = sim.run();
    verify(tester, success, "The lower case of A is a", total_points);
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

void setup(Tester & tester)
{
    tester.registerTest("Upper Case", UpperCaseTest, 40, false);
    tester.registerTest("Upper Case", UpperCaseTest, 10, true);
    tester.registerTest("Lower Case", LowerCaseTest, 40, false);
    tester.registerTest("Lower Case", LowerCaseTest, 10, true);
}

void shutdown(void) {}
