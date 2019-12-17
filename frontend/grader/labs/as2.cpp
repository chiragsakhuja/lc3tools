#include <algorithm>
#include <iomanip>
#include <sstream>

#include "../framework2.h"

static constexpr uint64_t InstLimit = 50000;
static constexpr uint16_t StartAddr = 0x32F0;

void setupMem(lc3::sim & sim, std::vector<int16_t> const & data)
{
    uint16_t addr = StartAddr;
    for(int16_t x : data) {
        sim.setMem(addr, x);
        ++addr;
    }
}

void verify(bool success, lc3::sim & sim, std::vector<int16_t> const & data, std::string const & message,
    double points, Grader & grader)
{
    if(! success) { grader.error("Execution hit exception"); return; }
    if(sim.didExceedInstLimit()) { grader.error("Exceeded instruction limit"); return; }

    std::vector<int16_t> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    bool full_match = true;
    uint16_t addr = StartAddr;
    for(int16_t expected : sorted) {
        int16_t actual = sim.getMem(addr);
        std::stringstream stream;
        stream << std::hex;
        stream << "[x" << addr << "]";
        stream << " Actual: x" << std::setfill('0') << std::setw(4) << actual;
        stream << "; Expected: x" << std::setfill('0') << std::setw(4) << expected;
        grader.output(stream.str());

        full_match &= actual == expected;
        ++addr;
    }

    if(full_match) {
        grader.verify(message, full_match, points);
    }
}

void ExampleTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        0xFFFF,
        0x0062,
        0x0A73,
        0x006C,
        0x0070,
        0x0001,
        0x0063,
        0x0065,
        0x0062,
        0x0073,
        0x006E,
        0x006B,
        0xFF76,
        0x0F7A,
        0x0068,
        0x006D
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();
    verify(success, sim, data, "Sorted provided example", total_points, grader);
}

void PositiveTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        0x0010,
        0x000F,
        0x000E,
        0x000D,
        0x000C,
        0x000B,
        0x000A,
        0x0009,
        0x0008,
        0x0007,
        0x0006,
        0x0005,
        0x0004,
        0x0003,
        0x0002,
        0x0001
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();
    verify(success, sim, data, "Sorted positive numbers", total_points, grader);
}

void NegativeTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        0xFFFF,
        0xFFFE,
        0xFFFD,
        0xFFFC,
        0xFFFB,
        0xFFFA,
        0xFFF9,
        0xFFF8,
        0xFFF7,
        0xFFF6,
        0xFFF5,
        0xFFF4,
        0xFFF3,
        0xFFF2,
        0xFFF1,
        0xFFF0
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();
    verify(success, sim, data, "Sorted negative numbers", total_points, grader);
}

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();
    verify(success, sim, data, "Sorted zeroes", total_points, grader);
}

void NoChangeTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        0xFFF8,
        0xFFF9,
        0xFFFA,
        0xFFFB,
        0xFFFC,
        0xFFFD,
        0xFFFE,
        0xFFFF,
        0x0000,
        0x0001,
        0x0002,
        0x0003,
        0x0004,
        0x0005,
        0x0006,
        0x0007
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();

    verify(success, sim, data, "Did not change already-sorted list", total_points, grader);
}

void OverflowTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        0x7FFF,
        0x8000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();

    verify(success, sim, data, "Sorted overflowing numbers", total_points, grader);
}

void testBringup(lc3::sim & sim)
{
    sim.setPC(0x3000);
    sim.setRunInstLimit(InstLimit);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Grader & grader)
{
    grader.registerTest("Example", ExampleTest, 20, false);
    grader.registerTest("Example", ExampleTest, 10, true);
    grader.registerTest("Positive", PositiveTest, 10, false);
    grader.registerTest("Negative", NegativeTest, 10, false);
    grader.registerTest("Zero", ZeroTest, 5, false);
    grader.registerTest("Zero", ZeroTest, 5, true);
    grader.registerTest("NoChange", NoChangeTest, 30, false);
    grader.registerTest("Overflow", OverflowTest, 10, false);
}

void shutdown(void) {}
