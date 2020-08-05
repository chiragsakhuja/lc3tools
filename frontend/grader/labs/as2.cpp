#include <algorithm>
#include <iomanip>
#include <sstream>

#include "framework2.h"

static constexpr uint64_t InstLimit = 50000;
static constexpr uint16_t StartAddr = 0x32F0;

void setupMem(lc3::sim & sim, std::vector<int16_t> const & data)
{
    uint16_t addr = StartAddr;
    for(int16_t x : data) {
        sim.writeMem(addr, x);
        ++addr;
    }
}

void verify(bool success, lc3::sim & sim, std::vector<int16_t> const & data, std::string const & label,
    double points, Grader & grader)
{
    if(! success) { grader.error(label, "Execution hit exception"); return; }
    if(sim.didExceedInstLimit()) { grader.error(label, "Exceeded instruction limit"); return; }

    std::vector<int16_t> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    bool full_match = true;
    uint16_t addr = StartAddr;
    for(int16_t expected : sorted) {
        int16_t actual = sim.readMem(addr);
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
        grader.verify(label, full_match, points);
    }
}

void ExampleTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        static_cast<int16_t>(0xFFFF),
        static_cast<int16_t>(0x0062),
        static_cast<int16_t>(0x0A73),
        static_cast<int16_t>(0x006C),
        static_cast<int16_t>(0x0070),
        static_cast<int16_t>(0x0001),
        static_cast<int16_t>(0x0063),
        static_cast<int16_t>(0x0065),
        static_cast<int16_t>(0x0062),
        static_cast<int16_t>(0x0073),
        static_cast<int16_t>(0x006E),
        static_cast<int16_t>(0x006B),
        static_cast<int16_t>(0xFF76),
        static_cast<int16_t>(0x0F7A),
        static_cast<int16_t>(0x0068),
        static_cast<int16_t>(0x006D)
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
        static_cast<int16_t>(0xFFFF),
        static_cast<int16_t>(0xFFFE),
        static_cast<int16_t>(0xFFFD),
        static_cast<int16_t>(0xFFFC),
        static_cast<int16_t>(0xFFFB),
        static_cast<int16_t>(0xFFFA),
        static_cast<int16_t>(0xFFF9),
        static_cast<int16_t>(0xFFF8),
        static_cast<int16_t>(0xFFF7),
        static_cast<int16_t>(0xFFF6),
        static_cast<int16_t>(0xFFF5),
        static_cast<int16_t>(0xFFF4),
        static_cast<int16_t>(0xFFF3),
        static_cast<int16_t>(0xFFF2),
        static_cast<int16_t>(0xFFF1),
        static_cast<int16_t>(0xFFF0)
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();
    verify(success, sim, data, "Sorted negative numbers", total_points, grader);
}

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000)
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();
    verify(success, sim, data, "Sorted zeroes", total_points, grader);
}

void NoChangeTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        static_cast<int16_t>(0xFFF8),
        static_cast<int16_t>(0xFFF9),
        static_cast<int16_t>(0xFFFA),
        static_cast<int16_t>(0xFFFB),
        static_cast<int16_t>(0xFFFC),
        static_cast<int16_t>(0xFFFD),
        static_cast<int16_t>(0xFFFE),
        static_cast<int16_t>(0xFFFF),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0001),
        static_cast<int16_t>(0x0002),
        static_cast<int16_t>(0x0003),
        static_cast<int16_t>(0x0004),
        static_cast<int16_t>(0x0005),
        static_cast<int16_t>(0x0006),
        static_cast<int16_t>(0x0007)
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();

    verify(success, sim, data, "Did not change already-sorted list", total_points, grader);
}

void OverflowTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<int16_t> data = {
        static_cast<int16_t>(0x7FFF),
        static_cast<int16_t>(0x8000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000),
        static_cast<int16_t>(0x0000)
    };

    setupMem(sim, data);

    bool success = sim.runUntilHalt();

    verify(success, sim, data, "Sorted overflowing numbers", total_points, grader);
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
