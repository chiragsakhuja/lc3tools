/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cmath>
#include <array>
#include <vector>
#include <string>
#include <sstream>

#include "framework.h"

static constexpr double correct_thresh = 1.0 - 0.00001;
static constexpr double close_thresh = 0.9;
static constexpr double partial_thresh = 0.2;

std::string nimGolden(std::vector<std::array<char, 2>> const & inputs)
{
    int64_t state[] = {3, 5, 8};
    uint64_t player = 1;

    uint64_t input_pos = 0;
    std::stringstream output;
    while(input_pos < inputs.size()) {
        for(uint64_t i = 0; i < 3; i += 1) {
            output << "ROW " << static_cast<char>(i + 'A') << ": ";
            for(int64_t j = 0; j < state[i]; j += 1) {
                output << 'o';
            }
            output << '\n';
        }

        bool valid_move;
        char row, num;
        do {
            output << "Player " << player << ", choose a row and number of rocks: ";
            if(input_pos >= inputs.size()) {
                break;
            }
            row = inputs[input_pos][0];
            num = inputs[input_pos][1] - '0';
            output << row << static_cast<char>(num + '0') << '\n';
            valid_move = ('A' <= row && row <= 'C') && (0 < num && num <= state[row - 'A']);
            if(! valid_move) {
                output << "Invalid move. Try again.\n";
            }
            ++input_pos;
        } while(! valid_move);
        output << '\n';

        player = (player % 2) + 1;
        state[row - 'A'] -= num;

        if(state[0] + state[1] + state[2] == 0) {
            output << "Player " << player << " Wins.";
            break;
        }
    }

    std::vector<char> output_buffer;
    for(std::string line; std::getline(output, line); ) {
        for(char x : line) {
            output_buffer.push_back(x);
        }
        output_buffer.push_back('\n');
    }

    return std::string{output_buffer.begin(), output_buffer.end()};
}

std::string flatten(std::vector<std::array<char, 2>> const & inputs)
{
    std::stringstream ss;
    for(auto const & line : inputs) {
        ss << line[0] << line[1];
    }
    return ss.str();
}

std::ostream & operator<<(std::ostream & out, std::vector<char> const & buffer)
{
    for(char x : buffer) {
        out << x;
    }

    return out;
}

void verify(Tester & tester, bool success, std::string const & expected, std::string const & actual, double points)
{
    if(! success) { tester.error("Error", "Execution hit exception"); return; }

    double similarity = tester.checkSimilarity(expected, actual);
    tester.verify("Correct", similarity >= correct_thresh, points);
    if(similarity < correct_thresh) {
        tester.verify("Close enough", similarity >= close_thresh, points);
        if(similarity < close_thresh) {
            tester.verify("Partially correct", similarity >= partial_thresh, std::round(similarity * points));
        }
    }
}

void ExampleTest(lc3::sim & sim, Tester & tester, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'B', '2' },
        { 'A', '1' },
        { 'C', '6' },
        { 'G', '1' },
        { 'B', '3' },
        { 'A', '3' },
        { 'C', '2' },
        { 'A', '1' },
        { 'A', '*' },
        { '&', '4' },
        { 'A', '1' }
    };

    std::string input_str = flatten(inputs);
    tester.setInputString(input_str);

    sim.setRunInstLimit(50000);
    bool success = sim.runUntilHalt();
    std::string expected = nimGolden(inputs);

    uint64_t preprocess_type = Tester::PreprocessType::IgnoreCase |
        Tester::PreprocessType::IgnoreWhitespace |
        Tester::PreprocessType::IgnorePunctuation;
    std::string expected_pp = tester.getPreprocessedString(expected, preprocess_type);
    std::string actual_pp = tester.getPreprocessedString(tester.getOutput(), preprocess_type);
    verify(tester, success, expected_pp, actual_pp, total_points);
}


void CloseRowTest(lc3::sim & sim, Tester & tester, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { '@', '2' },
        { 'D', '1' }
    };

    std::string input_str = flatten(inputs);
    tester.setInputString(input_str);

    sim.setRunInstLimit(10000);
    bool success = sim.runUntilHalt();
    std::string expected = nimGolden(inputs);

    uint64_t preprocess_type = Tester::PreprocessType::IgnoreCase |
        Tester::PreprocessType::IgnoreWhitespace |
        Tester::PreprocessType::IgnorePunctuation;
    std::string expected_pp = tester.getPreprocessedString(expected, preprocess_type);
    std::string actual_pp = tester.getPreprocessedString(tester.getOutput(), preprocess_type);
    verify(tester, success, expected_pp, actual_pp, total_points);
}

void LowerCaseRowTest(lc3::sim & sim, Tester & tester, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'a', '2' }
    };

    std::string input_str = flatten(inputs);
    tester.setInputString(input_str);

    sim.setRunInstLimit(10000);
    bool success = sim.runUntilHalt();
    std::string expected = nimGolden(inputs);

    uint64_t preprocess_type = Tester::PreprocessType::IgnoreCase |
        Tester::PreprocessType::IgnoreWhitespace |
        Tester::PreprocessType::IgnorePunctuation;
    std::string expected_pp = tester.getPreprocessedString(expected, preprocess_type);
    std::string actual_pp = tester.getPreprocessedString(tester.getOutput(), preprocess_type);
    verify(tester, success, expected_pp, actual_pp, total_points);
}

void CloseCountTest(lc3::sim & sim, Tester & tester, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'A', '4' },
        { 'A', '1' },
        { 'A', '3' },
        { 'A', '1' },
        { 'A', '2' },
        { 'A', '1' },
        { 'A', '1' }
    };

    std::string input_str = flatten(inputs);
    tester.setInputString(input_str);

    sim.setRunInstLimit(50000);
    bool success = sim.runUntilHalt();
    std::string expected = nimGolden(inputs);

    uint64_t preprocess_type = Tester::PreprocessType::IgnoreCase |
        Tester::PreprocessType::IgnoreWhitespace |
        Tester::PreprocessType::IgnorePunctuation;
    std::string expected_pp = tester.getPreprocessedString(expected, preprocess_type);
    std::string actual_pp = tester.getPreprocessedString(tester.getOutput(), preprocess_type);
    verify(tester, success, expected_pp, actual_pp, total_points);
}

void ZeroCountTest(lc3::sim & sim, Tester & tester, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'A', '0' }
    };

    std::string input_str = flatten(inputs);
    tester.setInputString(input_str);

    sim.setRunInstLimit(10000);
    bool success = sim.runUntilHalt();
    std::string expected = nimGolden(inputs);

    uint64_t preprocess_type = Tester::PreprocessType::IgnoreCase |
        Tester::PreprocessType::IgnoreWhitespace |
        Tester::PreprocessType::IgnorePunctuation;
    std::string expected_pp = tester.getPreprocessedString(expected, preprocess_type);
    std::string actual_pp = tester.getPreprocessedString(tester.getOutput(), preprocess_type);
    verify(tester, success, expected_pp, actual_pp, total_points);
}


void ExactOutputTest(lc3::sim & sim, Tester & tester, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'B', '2' },
        { 'A', '1' },
        { 'C', '6' },
        { 'G', '1' },
        { 'B', '3' },
        { 'A', '3' },
        { 'C', '2' },
        { 'A', '1' },
        { 'A', '*' },
        { '&', '4' },
        { 'A', '1' }
    };

    std::string input_str = flatten(inputs);
    tester.setInputString(input_str);

    sim.setRunInstLimit(50000);
    bool success = sim.runUntilHalt();

    std::string output = tester.getOutput();
    std::string expected = nimGolden(inputs);

    uint64_t preprocess_type = Tester::PreprocessType::IgnoreCase |
        Tester::PreprocessType::IgnoreWhitespace |
        Tester::PreprocessType::IgnorePunctuation;
    auto expected_all = tester.getPreprocessedString(expected, preprocess_type);
    auto actual_all = tester.getPreprocessedString(output, preprocess_type);
    double similarity = tester.checkSimilarity(expected_all, actual_all);
    tester.verify("Correct behavior", similarity >= correct_thresh, 0);

    if(similarity >= correct_thresh) {
        preprocess_type = Tester::PreprocessType::IgnoreWhitespace | Tester::PreprocessType::IgnorePunctuation;
        auto expected_c = tester.getPreprocessedString(expected, preprocess_type);
        auto actual_c = tester.getPreprocessedString(output, preprocess_type);

        preprocess_type = Tester::PreprocessType::IgnoreCase | Tester::PreprocessType::IgnorePunctuation;
        auto expected_w = tester.getPreprocessedString(expected, preprocess_type);
        auto actual_w = tester.getPreprocessedString(output, preprocess_type);

        preprocess_type = Tester::PreprocessType::IgnoreWhitespace | Tester::PreprocessType::IgnoreCase;
        auto expected_p = tester.getPreprocessedString(expected, preprocess_type);
        auto actual_p = tester.getPreprocessedString(output, preprocess_type);

        if(! success) { tester.error("Error", "Execution hit exception"); return; }
        if(sim.didExceedInstLimit()) { tester.error("Error", "Exceeded instruction limit"); return; }

        tester.verify("Correct capitalization", tester.checkSimilarity(expected_c, actual_c) >= correct_thresh,
            std::round(total_points / 3.0));
        tester.verify("Correct whitespace", tester.checkSimilarity(expected_w, actual_w) >= correct_thresh,
            std::round(total_points / 3.0));
        tester.verify("Correct punctuation", tester.checkSimilarity(expected_p, actual_p) >= correct_thresh,
            std::round(total_points / 3.0));
    }
}

void testBringup(lc3::sim & sim)
{
    sim.writePC(0x3000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Tester & tester)
{
    tester.registerTest("ExampleTest", ExampleTest, 15, false);
    tester.registerTest("ExampleTest", ExampleTest, 15, true);
    tester.registerTest("CloseRow", CloseRowTest, 10, false);
    tester.registerTest("LowerCaseRow", LowerCaseRowTest, 10, false);
    tester.registerTest("CloseCount", CloseCountTest, 10, false);
    tester.registerTest("ZeroCount", ZeroCountTest, 10, false);
    tester.registerTest("ExactOutput", ExactOutputTest, 30, false);
}

void shutdown(void) {}
