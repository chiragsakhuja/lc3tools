#include "framework2.h"

static constexpr uint64_t InstLimit = 1000;

static constexpr uint64_t shift(uint64_t value, uint64_t amount)
{
    return (value << amount) & 0xFFFF;
}

void verify(bool success, bool exceeded_inst_limit, uint32_t expected, uint32_t actual, std::string const & message,
    double points, Grader & grader)
{
    std::stringstream stream;
    stream << "Expected: " << expected << "; Actual: " << actual;
    grader.output(stream.str());

    if(success) {
        if(! exceeded_inst_limit) {
            grader.verify(message, actual == expected, points);
        } else {
            grader.error("exceeded instruction limit");
        }
    } else {
        grader.error("exection failed");
    }
}

void OneByOneTest(lc3::sim & sim, Grader & grader, double total_points)
{
    uint64_t value = 0x1;
    uint64_t amount = 0x1;
    sim.writeMem(0x3100, value);
    sim.writeMem(0x3101, amount);

    bool success = sim.runUntilHalt();
    bool exceeded_inst_limit = sim.didExceedInstLimit();
    uint32_t expected = sim.readMem(0x3102);
    uint32_t actual = shift(value, amount);
    verify(success, exceeded_inst_limit, expected, actual, "OneByOne", total_points, grader);
}

void AllByOneTest(lc3::sim & sim, Grader & grader, double total_points)
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
        uint32_t expected = sim.readMem(0x3102);
        uint32_t actual = shift(value, amount);
        verify(success, exceeded_inst_limit, expected, actual, "AllByOne", total_points / 15, grader);
    }
}

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    uint64_t values[] = {0, 1};
    uint64_t amounts[] = {0, 1};

    for(uint64_t i = 0; i < 1; i += 1) {
        for(uint64_t j = 0; j < 1; j += 1) {
            sim.writePC(0x3000);
            sim.setRunInstLimit(InstLimit);

            sim.writeMem(0x3100, values[i]);
            sim.writeMem(0x3101, amounts[j]);

            bool success = sim.runUntilHalt();
            bool exceeded_inst_limit = sim.didExceedInstLimit();
            uint32_t expected = sim.readMem(0x3102);
            uint32_t actual = shift(values[i], amounts[j]);
            verify(success, exceeded_inst_limit, expected, actual, "Zero", total_points, grader);
        }
    }
}

void OnesTest(lc3::sim & sim, Grader & grader, double total_points)
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
        uint32_t expected = sim.readMem(0x3102);
        uint32_t actual = shift(value, amount);
        verify(success, exceeded_inst_limit, expected, actual, "Ones", total_points / 17, grader);
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

void setup(Grader & grader)
{
    grader.registerTest("OneByOne", OneByOneTest, 10, false);
    grader.registerTest("AllByOne", AllByOneTest, 40, false);
    grader.registerTest("AllByOne", AllByOneTest, 10, true);
    grader.registerTest("Zero", ZeroTest, 20, false);
    grader.registerTest("Zero", ZeroTest, 10, true);
    grader.registerTest("Ones", OnesTest, 10, false);
}

void shutdown(void) {}
