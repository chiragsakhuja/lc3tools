/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

static constexpr uint64_t InstLimit = 1000;

static constexpr uint64_t rotate(uint64_t value, uint64_t amount)
{
    return ((value << amount) | (value >> (16 - amount))) & 0xFFFF;
}

void verify(bool success, bool exceeded_inst_limit, uint32_t expected, uint32_t actual, std::string const & label,
    double points, Tester & tester)
{
    std::stringstream stream;
    stream << "Expected: " << expected << "; Actual: " << actual;
    tester.output(stream.str());

    if(success) {
        if(! exceeded_inst_limit) {
            tester.verify(label, actual == expected, points);
        } else {
            tester.error(label, "Exceeded instruction limit");
        }
    } else {
        tester.error(label, "Execution failed");
    }
}

void OneByOneTest(lc3::sim & sim, Tester & tester, double total_points)
{
    uint64_t value = 0x8000;
    uint64_t amount = 0x1;
    sim.writeMem(0x3100, value);
    sim.writeMem(0x3101, amount);

    bool success = sim.runUntilHalt();
    bool exceeded_inst_limit = sim.didExceedInstLimit();
    uint32_t expected = rotate(value, amount);
    uint32_t actual = sim.readMem(0x3102);

    verify(success, exceeded_inst_limit, expected, actual, "OneByOne", total_points, tester);
}

void AllByOneTest(lc3::sim & sim, Tester & tester, double total_points)
{
    for(uint64_t i = 1; i < 16; i += 1) {
        sim.writePC(0x3000);
        sim.setRunInstLimit(InstLimit);

        uint64_t value = 0x1 << i;
        uint64_t amount = 0x1;
        sim.writeMem(0x3100, value);
        sim.writeMem(0x3101, amount);

        bool success = sim.runUntilHalt();
        bool exceeded_inst_limit = sim.didExceedInstLimit();
        uint32_t expected = rotate(value, amount);
        uint32_t actual = sim.readMem(0x3102);

        verify(success, exceeded_inst_limit, expected, actual, "AllByOne", total_points / 15, tester);
    }
}

void ZeroTest(lc3::sim & sim, Tester & tester, double total_points)
{
    uint64_t values[] = {0, 0x8000};
    uint64_t amounts[] = {0, 1};

    for(uint64_t i = 0; i < 1; i += 1) {
        for(uint64_t j = 0; j < 1; j += 1) {
            sim.writePC(0x3000);
            sim.setRunInstLimit(InstLimit);

            sim.writeMem(0x3100, values[i]);
            sim.writeMem(0x3101, amounts[j]);

            bool success = sim.runUntilHalt();
            bool exceeded_inst_limit = sim.didExceedInstLimit();
            uint32_t expected = rotate(values[i], amounts[j]);
            uint32_t actual = sim.readMem(0x3102);

            verify(success, exceeded_inst_limit, expected, actual, "Zero", total_points, tester);
        }
    }
}

void OnesTest(lc3::sim & sim, Tester & tester, double total_points)
{
    for(uint64_t i = 0; i <= 16; i += 1) {
        sim.writePC(0x3000);
        sim.setRunInstLimit(InstLimit);

        uint64_t value = 0xFFFF;
        uint64_t amount = i;
        sim.writeMem(0x3100, value);
        sim.writeMem(0x3101, amount);

        bool success = sim.runUntilHalt();
        bool exceeded_inst_limit = sim.didExceedInstLimit();
        uint32_t expected = rotate(value, amount);
        uint32_t actual = sim.readMem(0x3102);

        verify(success, exceeded_inst_limit, expected, actual, "Ones", total_points / 17, tester);
    }
}

void testBringup(lc3::sim & sim)
{
    sim.writePC(0x3000);
    sim.setRunInstLimit(InstLimit);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Tester & tester)
{
    tester.registerTest("OneByOne", OneByOneTest, 10, false);
    tester.registerTest("AllByOne", AllByOneTest, 40, false);
    tester.registerTest("AllByOne", AllByOneTest, 10, true);
    tester.registerTest("Zero", ZeroTest, 20, false);
    tester.registerTest("Zero", ZeroTest, 10, true);
    tester.registerTest("Ones", OnesTest, 10, false);
}

void shutdown(void) {}
