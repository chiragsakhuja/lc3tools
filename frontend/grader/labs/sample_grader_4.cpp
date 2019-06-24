#include "../framework.h"

static constexpr uint64_t InstLimit = 100000;

void OneTest(lc3::sim & sim, StringInputter & inputter)
{
    // Setup
    sim.runUntilInputPoll();
    VERIFY_OUTPUT("Type a professor's name and then press enter:");

    inputter.setString("Dan\n");
    sim.runUntilInputPoll();
    VERIFY_OUTPUT_HAD("16000");

/*
 *    inputter.setString("Dani\n");
 *    sim.runUntilInputPoll();
 *    VERIFY_OUTPUT_HAD("No Entry");
 *
 *    inputter.setString("Daniel");
 *    sim.runUntilInputPoll();
 *    VERIFY_OUTPUT_HAD("24000");
 *
 *    inputter.setString("dan");
 *    sim.runUntilInputPoll();
 *    VERIFY_OUTPUT_HAD("No Entry");
 */
    inputter.setString("d");

    bool success = sim.runUntilHalt();
    VERIFY(success && sim.getInstExecCount() < InstLimit);
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

void setup(void)
{
    REGISTER_TEST(One, OneTest, 60);
}

void shutdown(void) {}
