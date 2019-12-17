#include <cmath>
#include <vector>
#include <string>
#include <sstream>

#include "../framework2.h"

static constexpr double correct_thresh = 1.0 - 0.00001;
static constexpr double close_thresh = 0.9;
static constexpr double partial_thresh = 0.2;

std::vector<char> nimGolden(std::vector<std::array<char, 2>> const & inputs)
{
    int64_t state[] = {3, 5, 8};
    uint64_t player = 1;

    uint64_t input_pos = 0;
    std::stringstream output;
    while(input_pos < inputs.size()) {
        for(uint64_t i = 0; i < 3; i += 1) {
            output << "ROW " << static_cast<char>(i + 'A') << ": ";
            for(uint64_t j = 0; j < state[i]; j += 1) {
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

    return output_buffer;
}

enum PreprocessType {
    IgnoreCase = 1,
    IgnoreWhitespace = 2,
    IgnorePunctuation = 4
};

std::string flatten(std::vector<std::array<char, 2>> const & inputs)
{
    std::stringstream ss;
    for(auto const & line : inputs) {
        ss << line[0] << line[1];
    }
    return ss.str();
}

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

std::vector<char> & preprocess(std::vector<char> & buffer, uint64_t type)
{
    // Always remove trailing whitespace
    for(uint64_t i = 0; i < buffer.size(); i += 1) {
        if(buffer[i] == '\n') {
            int64_t pos = i - 1;
            while(pos >= 0 && std::isspace(buffer[pos])) {
                buffer.erase(buffer.begin() + pos);
                if(pos == 0 || buffer[pos - 1] == '\n') { break; }
                --pos;
            }
        }
    }

    // Always remove new lines at end of file
    for(int64_t i = buffer.size() - 1; i >= 0 && std::isspace(buffer[i]); --i) {
        buffer.erase(buffer.begin() + i);
    }

    // Remove other characters
    for(uint64_t i = 0; i < buffer.size(); i += 1) {
        if((type & PreprocessType::IgnoreCase) && 'A' <= buffer[i] && buffer[i] <= 'Z') {
            buffer[i] |= 0x20;
        } else if(((type & PreprocessType::IgnoreWhitespace) && std::isspace(buffer[i])) ||
                  ((type & PreprocessType::IgnorePunctuation) && std::ispunct(buffer[i])))
        {
            buffer.erase(buffer.begin() + i);
            --i;
        }
    }
    return buffer;
}

std::ostream & operator<<(std::ostream & out, std::vector<char> const & buffer)
{
    for(char x : buffer) {
        //out << static_cast<uint64_t>(x) << ' ';
        out << x;
    }

    return out;
}

void verify(Grader & grader, bool success, std::vector<char> const & expected, std::vector<char> const & actual,
    double points)
{
    if(! success) { grader.error("Execution hit exception"); return; }

    double similarity = compareOutput(expected, actual);
    /*
     *std::cout << "Actual: \n" << actual << '\n';
     *std::cout << "Expected: \n" << expected << '\n';
     *std::cout << "Similarity: " << similarity << '\n';
     */
    grader.verify("Correct", similarity >= correct_thresh, points);
    if(similarity < correct_thresh) {
        grader.verify("Close enough", similarity >= close_thresh, points);
        if(similarity < close_thresh) {
            grader.verify("Partially correct", similarity >= partial_thresh, std::round(similarity * points));
        }
    }
}

void ExampleTest(lc3::sim & sim, Grader & grader, double total_points)
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
    grader.getInputter().setString(input_str);

    sim.setRunInstLimit(50000);
    bool success = sim.runUntilHalt();
    std::vector<char> expected = nimGolden(inputs);
    std::vector<char> actual = grader.getOutputter().display_buffer;

    uint64_t preprocess_type = PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation;
    verify(grader, success, preprocess(expected, preprocess_type), preprocess(actual, preprocess_type), total_points);
}


void CloseRowTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { '@', '2' },
        { 'D', '1' }
    };

    std::string input_str = flatten(inputs);
    grader.getInputter().setString(input_str);

    sim.setRunInstLimit(10000);
    bool success = sim.runUntilHalt();
    std::vector<char> expected = nimGolden(inputs);
    std::vector<char> actual = grader.getOutputter().display_buffer;

    uint64_t preprocess_type = PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation;
    verify(grader, success, preprocess(expected, preprocess_type), preprocess(actual, preprocess_type), total_points);
}

void LowerCaseRowTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'a', '2' }
    };

    std::string input_str = flatten(inputs);
    grader.getInputter().setString(input_str);

    sim.setRunInstLimit(10000);
    bool success = sim.runUntilHalt();
    std::vector<char> expected = nimGolden(inputs);
    std::vector<char> actual = grader.getOutputter().display_buffer;

    uint64_t preprocess_type = PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation;
    verify(grader, success, preprocess(expected, preprocess_type), preprocess(actual, preprocess_type), total_points);
}

void CloseCountTest(lc3::sim & sim, Grader & grader, double total_points)
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
    grader.getInputter().setString(input_str);

    sim.setRunInstLimit(50000);
    bool success = sim.runUntilHalt();
    std::vector<char> expected = nimGolden(inputs);
    std::vector<char> actual = grader.getOutputter().display_buffer;

    uint64_t preprocess_type = PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation;
    verify(grader, success, preprocess(expected, preprocess_type), preprocess(actual, preprocess_type), total_points);
}

void ZeroCountTest(lc3::sim & sim, Grader & grader, double total_points)
{
    std::vector<std::array<char, 2>> inputs = {
        { 'A', '0' }
    };

    std::string input_str = flatten(inputs);
    grader.getInputter().setString(input_str);

    sim.setRunInstLimit(10000);
    bool success = sim.runUntilHalt();
    std::vector<char> expected = nimGolden(inputs);
    std::vector<char> actual = grader.getOutputter().display_buffer;

    uint64_t preprocess_type = PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation;
    verify(grader, success, preprocess(expected, preprocess_type), preprocess(actual, preprocess_type), total_points);
}


void ExactOutputTest(lc3::sim & sim, Grader & grader, double total_points)
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
    grader.getInputter().setString(input_str);

    sim.setRunInstLimit(50000);
    bool success = sim.runUntilHalt();
    std::vector<char> expected = nimGolden(inputs);
    std::vector<char> actual = grader.getOutputter().display_buffer;
    auto expected_all = expected;
    auto actual_all = actual;
    preprocess(expected_all, PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation);
    preprocess(actual_all, PreprocessType::IgnoreCase | PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation);
    double similarity = compareOutput(expected_all, actual_all);
    grader.verify("Correct behavior", similarity >= correct_thresh, 0);
    std::cout << expected_all << "\n\n" << actual_all;

    if(similarity >= correct_thresh) {
        auto expected_c = expected;
        auto actual_c = actual;
        auto expected_w = expected;
        auto actual_w = actual;
        auto expected_p = expected;
        auto actual_p = actual;
        preprocess(expected_c, PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation);
        preprocess(actual_c, PreprocessType::IgnoreWhitespace | PreprocessType::IgnorePunctuation);
        preprocess(expected_w, PreprocessType::IgnoreCase | PreprocessType::IgnorePunctuation);
        preprocess(actual_w, PreprocessType::IgnoreCase | PreprocessType::IgnorePunctuation);
        preprocess(expected_p, PreprocessType::IgnoreWhitespace | PreprocessType::IgnoreCase);
        preprocess(actual_p, PreprocessType::IgnoreWhitespace | PreprocessType::IgnoreCase);

        std::stringstream ss;

        if(! success) { grader.error("Execution hit exception"); return; }
        if(grader.getSimulator().didExceedInstLimit()) { grader.error("Exceeded instruction limit"); return; }

        grader.verify("Correct capitalization", compareOutput(expected_c, actual_c) >= correct_thresh, std::round(total_points / 3.0));
        grader.verify("Correct whitespace", compareOutput(expected_w, actual_w) >= correct_thresh, std::round(total_points / 3.0));
        grader.verify("Correct punctuation", compareOutput(expected_p, actual_p) >= correct_thresh, std::round(total_points / 3.0));
    }
}

void testBringup(lc3::sim & sim)
{
    sim.setPC(0x3000);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Grader & grader)
{
    grader.registerTest("Example Test", ExampleTest, 15, false);
    grader.registerTest("Example Test", ExampleTest, 15, true);
    grader.registerTest("Close Row", CloseRowTest, 10, false);
    grader.registerTest("Lower Case Row", LowerCaseRowTest, 10, false);
    grader.registerTest("Close Count", CloseCountTest, 10, false);
    grader.registerTest("Zero Count", ZeroCountTest, 10, false);
    grader.registerTest("Exact Output", ExactOutputTest, 30, false);
}

void shutdown(void) {}
