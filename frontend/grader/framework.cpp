#include <memory>

#include "common.h"
#include "console_printer.h"
#include "console_inputter.h"
#include "framework.h"

struct CLIArgs
{
    uint32_t print_output = false;
    uint32_t print_level = 0;
    uint32_t print_level_override = false;
};

void setup(void);
void shutdown(void);
void testBringup(lc3::sim & sim);
void testTeardown(lc3::sim & sim);

std::vector<TestCase> tests;
uint32_t verify_count;
uint32_t verify_valid;

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

void StringInputter::setString(std::string const & source)
{
    this->source = source;
    this->pos = 0;
}

bool StringInputter::getChar(char & c)
{
    if(pos == source.size()) {
        return false;
    }

    c = source[pos];
    pos += 1;
    return true;
}

int main(int argc, char * argv[])
{
    CLIArgs args;
    std::vector<std::pair<std::string, std::string>> parsed_args = parseCLIArgs(argc, argv);
    for(auto const & arg : parsed_args) {
        if(std::get<0>(arg) == "print-output") {
            args.print_output = true;
        } else if(std::get<0>(arg) == "print-level") {
            args.print_level = std::stoi(std::get<1>(arg));
            args.print_level_override = true;
            args.print_output = true;
        }
    }

    lc3::ConsolePrinter asm_printer;
    lc3::as assembler(asm_printer, args.print_level_override ? args.print_level : 0);
    lc3::conv converter(asm_printer, args.print_level_override ? args.print_level : 0);

    std::vector<std::string> obj_filenames;
    bool valid_program = true;
    for(int i = 1; i < argc; i += 1) {
        std::string filename(argv[i]);
        if(filename[0] != '-') {
            std::pair<bool, std::string> result;
            if(endsWith(filename, ".bin")) {
                result = converter.convertBin(filename);
            } else {
                result = assembler.assemble(filename);
            }

            if(! result.first) { valid_program = false; }
            obj_filenames.push_back(result.second);
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
            lc3::sim simulator(sim_printer, sim_inputter,
                args.print_level_override ? args.print_level : 1, true);

            testBringup(simulator);

            verify_count = 0;
            verify_valid = 0;

            total_possible_points += test.points;

            std::cout << "Test: " << test.name;
            if(test.randomize) {
                simulator.randomize();
                std::cout << " (Randomized Machine)";
            }
            std::cout << std::endl;
            for(std::string const & obj_filename : obj_filenames) {
                if(! simulator.loadObjectFile(obj_filename)) {
                    std::cout << "could not init simulator\n";
                    return 2;
                }
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

    std::cout << check << (substr ? " sub? " : " ?= ");
    for(uint32_t i = 0; i < buffered_printer.display_buffer.size(); i += 1) {
        std::cout << buffered_printer.display_buffer[i];
    }
    std::cout << '\n';

    if(substr) {
        bool match = false;
        for(uint32_t i = 0; i < check.size(); i += 1) {
            if(i + check.size() >= buffered_printer.display_buffer.size()) { return false; }

            match = true;
            for(uint32_t j = 0; j < check.size(); i += 1) {
                if(buffered_printer.display_buffer[i] != check[i]) {
                    match = false;
                    break;
                }
            }
            if(match) { break; }
        }

        return match;
    } else {
        if(buffered_printer.display_buffer.size() != check.size()) { return false; }

        for(uint32_t i = 0; i < check.size(); i += 1) {
            if(buffered_printer.display_buffer[i] != check[i]) { return false; }
        }
        return true;
    }
    return false;
}

