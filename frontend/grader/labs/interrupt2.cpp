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

double compareOutput(std::vector<char> const & source, std::vector<char> const & target)
{
    if(source.size() > target.size()) {
        return compareOutput(target, source);
    }

    std::size_t min_size = source.size(), max_size = target.size();
    std::vector<std::size_t> lev_dist(min_size + 1);

    for(std::size_t i = 0; i < min_size + 1; i += 1) {
        lev_dist[i] = i;
    }

    for(std::size_t j = 1; j < max_size + 1; j += 1) {
        std::size_t prev_diag = lev_dist[0];
        ++lev_dist[0];

        for(std::size_t i = 1; i < min_size + 1; i += 1) {
            std::size_t prev_diag_tmp = lev_dist[i];
            if(source[i - 1] == target[j - 1]) {
                lev_dist[i] = prev_diag;
            } else {
                lev_dist[i] = std::min(std::min(lev_dist[i - 1], lev_dist[i]), prev_diag) + 1;
            }
            prev_diag = prev_diag_tmp;
        }
    }

    return 1 - static_cast<double>(lev_dist[min_size]) / min_size;
}

void verify(Grader & grader, bool success, std::string const & expected, std::string const & label, bool not_present,
    double points)
{
    if(! success) { grader.error(label, "Execution hit exception"); return; }

    std::vector<char> const & actual = grader.getOutputter().getBuffer();

    uint64_t actual_pos = 0;
    bool found = false;
    while(! found && actual_pos < actual.size()) {
        uint64_t expected_pos = 0;
        uint64_t actual_pos_ahead = actual_pos;

        while(expected_pos < expected.size() && actual_pos_ahead < actual.size() &&
            expected[expected_pos] == actual[actual_pos_ahead])
        {
            ++actual_pos_ahead;
            ++expected_pos;
        }

        if(expected_pos == expected.size()) {
            found = true;
        }

        ++actual_pos;
    }

    grader.verify(label, found ^ not_present, points);
}

void ExampleTest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.getInputter().setStringAfter("5", 1000);
    bool success = sim.run();
    verify(grader, success, "55555", "Contains correct count", false, total_points / 2);
    verify(grader, success, "555555", "Does not contain incorrect count", true, total_points / 2);
}

void ZeroTest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.getInputter().setStringAfter("0", 1000);
    bool success = sim.run();
    verify(grader, success, "0", "Does not contain zero", true, total_points);
}

void PrevASCIITest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.getInputter().setStringAfter("@", 1000);
    bool success = sim.run();
    verify(grader, success, "@ is not a decimal digit", "Correct behavior", false, total_points);
}

void NextASCIITest(lc3::sim & sim, Grader & grader, double total_points)
{
    sim.writePC(0x0800);
    grader.getInputter().setStringAfter(":", 1000);
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

