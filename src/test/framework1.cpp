/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <memory>

#include "common.h"
#include "console_printer.h"
#include "console_inputter.h"
#include "framework1.h"

namespace framework1
{
struct CLIArgs
{
    uint32_t print_output = false;
    uint32_t asm_print_level = 0;
    uint32_t asm_print_level_override = false;
    uint32_t sim_print_level = 0;
    uint32_t sim_print_level_override = false;
    bool ignore_privilege = false;
    bool liberal_asm = false;
};

std::vector<TestCase> tests;
uint32_t verify_count;
uint32_t verify_valid;

std::function<void(void)> setup = nullptr;
std::function<void(void)> shutdown = nullptr;
std::function<void(lc3::sim &)> testBringup = nullptr;
std::function<void(lc3::sim &)> testTeardown = nullptr;
lc3::core::SymbolTable symbol_table;

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
        } else if(std::get<0>(arg) == "liberal-asm") {
            args.liberal_asm = true;
        } else if(std::get<0>(arg) == "h" || std::get<0>(arg) == "help") {
            std::cout << "usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "\n";
            std::cout << "  -h,--help              Print this message\n";
            std::cout << "  --print-output         Print program output\n";
            std::cout << "  --asm-print-level=N    Assembler output verbosity [0-9]\n";
            std::cout << "  --sim-print-level=N    Simulator output verbosity [0-9]\n";
            std::cout << "  --ignore-privilege     Ignore access violations\n";
            std::cout << "  --liberal-asm          Enable liberal assembly syntax\n";
            return 0;
        }
    }

    lc3::ConsolePrinter asm_printer;
    lc3::as assembler(asm_printer, args.asm_print_level_override ? args.asm_print_level : 0, args.liberal_asm);
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

    setup();

    uint32_t total_points_earned = 0;
    uint32_t total_possible_points = 0;

    if(valid_program) {
        for(TestCase const & test : tests) {
            BufferedPrinter sim_printer(args.print_output);
            StringInputter sim_inputter;
            lc3::sim simulator(sim_printer, sim_inputter, args.sim_print_level_override ? args.sim_print_level : 1);

            testBringup(simulator);

            verify_count = 0;
            verify_valid = 0;

            total_possible_points += test.points;

            std::cout << "Test: " << test.name;
            if(test.randomize) {
                simulator.randomizeState();
                std::cout << " (Randomized Machine)";
            }
            std::cout << std::endl;
            for(std::string const & obj_filename : obj_filenames) {
                if(! simulator.loadObjFile(obj_filename)) {
                    std::cout << "could not init simulator\n";
                    return 2;
                }
            }

            if(args.ignore_privilege) {
                simulator.setIgnorePrivilege(true);
            }

            try {
                test.test_func(simulator, sim_inputter);
            } catch(lc3::utils::exception const & e) {
                std::cout << "Test case ran into exception: " << e.what() << "\n";
                continue;
            }

            testTeardown(simulator);

            float percent_points_earned = ((float) verify_valid) / verify_count;
            uint32_t points_earned = (uint32_t) ( percent_points_earned * test.points);
            std::cout << "Test points earned: " << points_earned << "/" << test.points << " ("
                      << (percent_points_earned * 100) << "%)\n";
            std::cout << "==========\n";

            total_points_earned += points_earned;
        }
    }

    std::cout << "==========\n";
    float percent_points_earned;
    if(total_possible_points == 0) {
        percent_points_earned = 0;
    } else {
        percent_points_earned = ((float) total_points_earned) / total_possible_points;
    }
    std::cout << "Total points earned: " << total_points_earned << "/" << total_possible_points << " ("
              << (percent_points_earned * 100) << "%)\n";

    shutdown();

    return 0;
}

bool outputCompare(lc3::utils::IPrinter const & printer, std::string check, bool substr)
{
    BufferedPrinter const & buffered_printer = static_cast<BufferedPrinter const &>(printer);

    std::cout << "is '" << check << "' " << (substr ? "substring of" : "==") << " '";
    for(uint32_t i = 0; i < buffered_printer.getBuffer().size(); i += 1) {
        if(buffered_printer.getBuffer()[i] == '\n') {
            std::cout << "\\n";
        } else {
            std::cout << buffered_printer.getBuffer()[i];
        }
    }
    std::cout << "'\n";

    if(substr) {
        uint64_t buffer_pos = 0;
        while(buffer_pos + check.size() < buffered_printer.getBuffer().size()) {
            uint64_t check_pos;
            for(check_pos = 0; check_pos < check.size(); check_pos += 1) {
                if(check[check_pos] != buffered_printer.getBuffer()[buffer_pos + check_pos]) {
                    break;
                }
            }
            if(check_pos == check.size()) { return true; }
            buffer_pos += 1;
        }
        return false;
    } else {
        if(buffered_printer.getBuffer().size() != check.size()) { return false; }

        for(uint32_t i = 0; i < check.size(); i += 1) {
            if(buffered_printer.getBuffer()[i] != check[i]) { return false; }
        }
        return true;
    }
    return false;
}

};

lc3::core::SymbolTable const & getSymbolTable(void) { return framework1::symbol_table; }

