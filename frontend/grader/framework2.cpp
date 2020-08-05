/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <memory>
#include <math.h>

#include "common.h"
#include "console_printer.h"
#include "console_inputter.h"
#include "framework2.h"

struct CLIArgs
{
    uint32_t print_output = false;
    uint32_t asm_print_level = 0;
    uint32_t asm_print_level_override = false;
    uint32_t sim_print_level = 0;
    uint32_t sim_print_level_override = false;
    bool ignore_privilege = false;
    bool grader_verbose = false;
    std::vector<std::string> test_filter;
};

void setup(Grader & grader);
void shutdown(void);
void testBringup(lc3::sim & sim);
void testTeardown(lc3::sim & sim);

std::vector<TestCase> tests;

bool endsWith(std::string const & search, std::string const & suffix)
{
    if(suffix.size() > search.size()) { return false; }
    return std::equal(suffix.rbegin(), suffix.rend(), search.rbegin());
}

void BufferedPrinter::print(std::string const & string)
{
    std::copy(string.begin(), string.end(), std::back_inserter(display_buffer));
    if(print_output) {
        std::cout << string;
    }
}

void BufferedPrinter::newline(void)
{
    display_buffer.push_back('\n');
    if(print_output) {
        std::cout << "\n";
    }
}

void StringInputter::setStringAfter(std::string const & source, uint32_t inst_count)
{
    this->inst_delay = inst_count;
    this->cur_inst_delay = inst_count;
    this->source = source;
    this->pos = 0;
}

bool StringInputter::getChar(char & c)
{
    if(inst_delay > 0) {
        --inst_delay;
        return false;
    }

    if(pos == source.size()) {
        return false;
    }

    c = source[pos];
    ++pos;
    cur_inst_delay = inst_delay;
    return true;
}

int main(int argc, char * argv[])
{
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
        } else if(std::get<0>(arg) == "grader-verbose") {
            args.grader_verbose = true;
        } else if(std::get<0>(arg) == "test-filter") {
            args.test_filter.push_back(std::get<1>(arg));
        } else if(std::get<0>(arg) == "h" || std::get<0>(arg) == "help") {
            std::cout << "usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "\n";
            std::cout << "  -h,--help              Print this message\n";
            std::cout << "  --print-output         Print program output\n";
            std::cout << "  --asm-print-level=N    Assembler output verbosity [0-9]\n";
            std::cout << "  --sim-print-level=N    Simulator output verbosity [0-9]\n";
            std::cout << "  --ignore-privilege     Ignore access violations\n";
            std::cout << "  --grader-verbose       Output grader messages\n";
            std::cout << "  --test-filter=TEST     Only run TEST (can be repeated)\n";
            return 0;
        }
    }

    lc3::ConsolePrinter asm_printer;
    lc3::as assembler(asm_printer, args.asm_print_level_override ? args.asm_print_level : 0, false);
    lc3::conv converter(asm_printer, args.asm_print_level_override ? args.asm_print_level : 0);

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
                    result = assembler.assemble(filename);
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
        Grader grader(args.print_output, args.sim_print_level_override ? args.sim_print_level : 1,
            args.ignore_privilege, args.grader_verbose, obj_filenames);
        setup(grader);

        if(args.test_filter.size() == 0) {
            grader.gradeAll();
        } else {
            for(std::string const & test_name : args.test_filter) {
                grader.grade(test_name);
            }
        }

        shutdown();
    }

    return 0;
}

TestCase::TestCase(std::string const & name, test_func_t test_func, double points, bool randomize)
    : name(name), test_func(test_func), points(points), randomize(randomize)
{}

Grader::Grader(bool print_output, uint32_t print_level, bool ignore_privilege, bool verbose,
    std::vector<std::string> const & obj_filenames)
    : print_output(print_output), ignore_privilege(ignore_privilege), verbose(verbose),
      print_level(print_level), obj_filenames(obj_filenames)
{
    resetTestPoints();
}

void Grader::registerTest(std::string const & name, test_func_t test_func, double points, bool randomize)
{
    tests.emplace_back(name, test_func, points, randomize);
}

std::pair<double, double> Grader::gradeAll(void)
{
    double total_points_earned = 0, total_points = 0;
    for(TestCase const & test : tests) {
        auto points = grade(test);
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

std::pair<double, double> Grader::grade(std::string const & test_name)
{
    for(TestCase const & test : tests) {
        if(test.name == test_name) {
            return grade(test);
        }
    }

    return std::make_pair(0, 0);
}

std::pair<double, double> Grader::grade(TestCase const & test)
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
        simulator.randomizeState();
        std::cout << " (Randomized Machine)";
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

void Grader::verify(std::string const & label, bool pred, double points)
{
    std::cout << "  " << label << " => ";
    if(pred) {
        std::cout << "Correct (+" << points << " pts)";
        test_points_earned += points;
    } else {
        std::cout << "Incorrect (+0 pts)";
    }
    std::cout << std::endl;
}

void Grader::output(std::string const & message)
{
    if(verbose) {
        std::cout << "  " << message << "\n";
    }
}

void Grader::error(std::string const & label, std::string const & message)
{
    std::cout << "  " << label << " => " << message << " (+0 pts)\n";
}

void Grader::resetTestPoints(void)
{
    test_points_earned = 0;
}
