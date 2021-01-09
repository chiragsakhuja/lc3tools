/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <memory>
#include <math.h>

#include "common.h"
#include "console_printer.h"
#include "framework2.h"

namespace framework2
{
struct CLIArgs
{
    uint32_t print_output = false;
    uint32_t asm_print_level = 0;
    uint32_t asm_print_level_override = false;
    uint32_t sim_print_level = 0;
    uint32_t sim_print_level_override = false;
    bool ignore_privilege = false;
    bool tester_verbose = false;
    uint64_t seed = 0;
    std::vector<std::string> test_filter;
};

std::vector<TestCase> tests;

std::function<void(Tester &)> setup = nullptr;
std::function<void(void)> shutdown = nullptr;
std::function<void(lc3::sim &)> testBringup = nullptr;
std::function<void(lc3::sim &)> testTeardown = nullptr;

int main(int argc, char * argv[])
{
    if(setup == nullptr || shutdown == nullptr || testBringup == nullptr || testTeardown == nullptr) {
        // Will never happen if framework.h wrapper is used, since, if any of these functions are missing, linking
        // will fail.
        std::cerr << "Unit test does not implement necessary functionality.\n";
        return 0;
    }

    CLIArgs args;
    std::vector<std::pair<std::string, std::string>> parsed_args = parseCLIArgs(argc, argv);
    for(auto const & arg : parsed_args) {
        if(std::get<0>(arg) == "print-output") {
            args.print_output = true;
        } else if(std::get<0>(arg) == "asm-print-level") {
            args.asm_print_level = std::stoi(std::get<1>(arg));
            args.asm_print_level_override = true;
        } else if(std::get<0>(arg) == "sim-print-level") {
            args.sim_print_level = std::stoi(std::get<1>(arg));
            args.sim_print_level_override = true;
            args.print_output = true;
        } else if(std::get<0>(arg) == "ignore-privilege") {
            args.ignore_privilege = true;
        } else if(std::get<0>(arg) == "tester-verbose") {
            args.tester_verbose = true;
        } else if(std::get<0>(arg) == "seed") {
            args.seed = std::stoull(std::get<1>(arg));
        } else if(std::get<0>(arg) == "test-filter") {
            args.test_filter.push_back(std::get<1>(arg));
        } else if(std::get<0>(arg) == "h" || std::get<0>(arg) == "help") {
            std::cout << "usage: " << argv[0] << " [OPTIONS] FILE [FILE...]\n";
            std::cout << "\n";
            std::cout << "  -h,--help              Print this message\n";
            std::cout << "  --print-output         Print program output\n";
            std::cout << "  --asm-print-level=N    Assembler output verbosity [0-9]\n";
            std::cout << "  --sim-print-level=N    Simulator output verbosity [0-9]\n";
            std::cout << "  --ignore-privilege     Ignore access violations\n";
            std::cout << "  --tester-verbose       Output tester messages\n";
            std::cout << "  --seed=N               Optional seed for randomization\n";
            std::cout << "  --test-filter=TEST     Only run TEST (can be repeated)\n";
            return 0;
        }
    }

    lc3::ConsolePrinter asm_printer;
    lc3::as assembler(asm_printer, args.asm_print_level_override ? args.asm_print_level : 0, false);
    lc3::conv converter(asm_printer, args.asm_print_level_override ? args.asm_print_level : 0);
    lc3::core::SymbolTable symbol_table;

    std::vector<std::string> obj_filenames;
    bool valid_program = true;
    for(int i = 1; i < argc; i += 1) {
        std::string filename(argv[i]);
        if(filename[0] != '-') {
            lc3::optional<std::string> result;
            if(! endsWith(filename, ".obj")) {
                if(endsWith(filename, ".bin")) {
                    result = converter.convertBin(filename);
                } else {
                    lc3::optional<std::pair<std::string, lc3::core::SymbolTable>> asm_result;
                    asm_result = assembler.assemble(filename);
                    if(asm_result) {
                        symbol_table.insert(asm_result->second.begin(), asm_result->second.end());
                        result = asm_result->first;
                    }
                }
            } else {
                result = filename;
            }

            if(result) {
                obj_filenames.push_back(*result);
            } else {
                valid_program = false;
            }
        }
    }

    if(obj_filenames.size() == 0) {
        return 1;
    }

    if(valid_program) {
        Tester tester(args.print_output, args.sim_print_level_override ? args.sim_print_level : 1,
            args.ignore_privilege, args.tester_verbose, args.seed, obj_filenames);
        tester.setSymbolTable(symbol_table);
        setup(tester);

        if(args.test_filter.size() == 0) {
            tester.testAll();
        } else {
            for(std::string const & test_name : args.test_filter) {
                tester.testSingle(test_name);
            }
        }

        shutdown();
    }

    return 0;
}

TestCase::TestCase(std::string const & name, test_func_t test_func, double points, bool randomize)
    : name(name), test_func(test_func), points(points), randomize(randomize)
{}

Tester::Tester(bool print_output, uint32_t print_level, bool ignore_privilege, bool verbose,
    uint64_t seed, std::vector<std::string> const & obj_filenames)
    : print_output(print_output), ignore_privilege(ignore_privilege), verbose(verbose),
      print_level(print_level), seed(seed), obj_filenames(obj_filenames)
{
    resetTestPoints();
}

void Tester::registerTest(std::string const & name, test_func_t test_func, double points, bool randomize)
{
    tests.emplace_back(name, test_func, points, randomize);
}

std::pair<double, double> Tester::testAll(void)
{
    double total_points_earned = 0, total_points = 0;
    for(TestCase const & test : tests) {
        auto points = testSingle(test);
        total_points_earned += std::get<0>(points);
        total_points += std::get<1>(points);
    }

    std::cout << "==========\n";
    std::cout << "==========\n";

    double percent_points_earned = total_points_earned / total_points;
    std::cout << "Total points earned: " << total_points_earned << "/" << total_points << " ("
              << (percent_points_earned * 100) << "%)\n";

    return std::make_pair(total_points_earned, total_points);
}

std::pair<double, double> Tester::testSingle(std::string const & test_name)
{
    for(TestCase const & test : tests) {
        if(test.name == test_name) {
            return testSingle(test);
        }
    }

    return std::make_pair(0, 0);
}

std::pair<double, double> Tester::testSingle(TestCase const & test)
{
    resetTestPoints();

    BufferedPrinter printer(print_output);
    StringInputter inputter;
    lc3::sim simulator(printer, inputter, print_level);
    this->printer = &printer;
    this->inputter = &inputter;
    this->simulator = &simulator;

    std::cout << "==========\n";
    std::cout << "Test: " << test.name;

    if(test.randomize) {
        if(seed == 0) {
            seed = simulator.randomizeState();
        } else {
            simulator.randomizeState(seed);
        }
        std::cout << " (Randomized Machine, Seed: " << seed << ")";
    }
    std::cout << std::endl;

    for(std::string const & obj_filename : obj_filenames) {
        if(! simulator.loadObjFile(obj_filename)) {
            std::cout << "Could not init simulator\n";
            return std::make_pair(0, test.points);
        }
    }

    testBringup(simulator);

    if(ignore_privilege) {
        simulator.setIgnorePrivilege(true);
    }

    try {
        test.test_func(simulator, *this, test.points);
    } catch(lc3::utils::exception const & e) {
        error("c++ exception", std::string(e.what()));
        std::cout << "Test case ran into exception: " << e.what() << "\n";
        return std::make_pair(0, test.points);
    }

    testTeardown(simulator);

    // In case the verify points don't add up to the total points, clamp
    double points_earned = std::min(test_points_earned, test.points);
    double percent_points_earned = points_earned / test.points;
    std::cout << "Test points earned: " << points_earned << "/" << test.points << " ("
              << (percent_points_earned * 100) << "%)\n";

    this->printer = nullptr;
    this->inputter = nullptr;
    this->simulator = nullptr;

    return std::make_pair(points_earned, test.points);
}

void Tester::verify(std::string const & label, bool pred, double points)
{
    std::cout << "  " << label << " => ";
    if(pred) {
        std::cout << "Pass (+" << points << " pts)";
        test_points_earned += points;
    } else {
        std::cout << "Fail (+0 pts)";
    }
    std::cout << std::endl;
}

void Tester::output(std::string const & message)
{
    if(verbose) {
        std::cout << "  " << message << "\n";
    }
}

void Tester::error(std::string const & label, std::string const & message)
{
    std::cout << "  " << label << " => " << message << " (+0 pts)\n";
}

void Tester::resetTestPoints(void)
{
    test_points_earned = 0;
}

std::string Tester::getOutput(void) const
{
    auto const & buffer = printer->getBuffer();
    return std::string{buffer.begin(), buffer.end()};
}

bool Tester::checkContain(std::string const & str, std::string const & expected_part) const
{
    if(expected_part.size() > str.size()) { return false; }

    for(uint64_t i = 0; i < str.size(); ++i) {
        uint64_t j;
        for(j = 0; j < expected_part.size() && i + j < str.size(); ++j) {
            if(str[i + j] != expected_part[j]) {
                break;
            }
        }
        if(j == expected_part.size()) { return true; }
    }

    return false;
}

double Tester::checkSimilarity(std::string const & source, std::string const & target) const
{
    std::vector<char> source_buffer{source.begin(), source.end()};
    std::vector<char> target_buffer{target.begin(), target.end()};
    return checkSimilarityHelper(source_buffer, target_buffer);
}

double Tester::checkSimilarityHelper(std::vector<char> const & source, std::vector<char> const & target) const
{
    if(source.size() > target.size()) {
        return checkSimilarityHelper(target, source);
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

std::string Tester::getPreprocessedString(std::string const & str, uint64_t type) const
{
    std::vector<char> buffer{str.begin(), str.end()};

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

    return std::string{buffer.begin(), buffer.end()};
}
};
