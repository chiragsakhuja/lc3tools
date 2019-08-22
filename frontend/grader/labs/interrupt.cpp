/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "../framework.h"

void UpperCaseTest(lc3::sim & sim, StringInputter & inputter)
{
    // This is a hack that will need to be replaced by adding an inputter.setStringAfter(str, inst_count) function
    inputter.setStringAfter("a", 50);
    bool success = sim.run();
    VERIFY_OUTPUT_HAD_NAMED("a", "a is not a capital letter of the English alphabet");
    VERIFY(success);
}

void LowerCaseTest(lc3::sim & sim, StringInputter & inputter)
{
    // This is a hack that will need to be replaced by adding an inputter.setStringAfter(str, inst_count) function
    inputter.setStringAfter("A", 50);
    bool success = sim.run();
    VERIFY_OUTPUT_HAD_NAMED("A", "The lower case of A is a");
    VERIFY(success);
}

void testBringup(lc3::sim & sim)
{
    sim.setPC(0x3000);
    sim.setRunInstLimit(10000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(void)
{
    REGISTER_TEST(UpperCase, UpperCaseTest, 40);
    REGISTER_RANDOM_TEST(UpperCase, UpperCaseTest, 10);
    REGISTER_TEST(LowerCase, LowerCaseTest, 40);
    REGISTER_RANDOM_TEST(LowerCase, LowerCaseTest, 10);
}

void shutdown(void) {}
