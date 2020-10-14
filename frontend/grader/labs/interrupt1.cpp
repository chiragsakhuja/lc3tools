/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

bool compareOutput(std::vector<char> const & source, std::vector<char> const & target)
{
    if(source.size() < target.size()) { return false; }

    bool found_match = false;
    for(uint64_t i = 0; i < source.size(); ++i) {
        found_match = true;
        for(uint64_t j = 0; j < target.size() && i + j < source.size(); ++j) {
            if(source[i + j] != target[j]) {
                found_match = false;
                break;
            }
        }
    }

    return found_match;
}

void verify(Grader & grader, bool success, std::string const & expected, std::vector<char> const & actual,
    double points)
{
    if(! success) { grader.error("Error", "Execution hit exception"); return; }

    std::vector<char> expected_v(expected.begin(), expected.end());
    bool found_match = compareOutput(actual, expected_v);
    grader.verify("Correct", found_match, points);
}

void UpperCaseTest(lc3::sim & sim, Grader & grader, double total_points)
{
    grader.getInputter().setStringAfter("a", 50);
    bool success = sim.run();
    verify(grader, success, "a is not a capital letter of the English alphabet", grader.getOutputter().getBuffer(),
        total_points);
}

void LowerCaseTest(lc3::sim & sim, Grader & grader, double total_points)
{
    grader.getInputter().setStringAfter("A", 50);
    bool success = sim.run();
    verify(grader, success, "The lower case of A is a", grader.getOutputter().getBuffer(), total_points);
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
