#include "framework.h"

void setup(void);

std::vector<TestCase> tests;
uint32_t verify_count;
uint32_t verify_valid;

void BufferedPrinter::print(std::string const & string)
{
    std::copy(string.begin(), string.end(), std::back_inserter(display_buffer));
}

void BufferedPrinter::newline(void) { display_buffer.push_back('\n'); }

bool FileInputter::getChar(char & c)
{
    return false;
}

StringInputter::StringInputter(std::string const & source)
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

int main(int argc, char ** argv)
{
    setup();

    uint32_t total_points_earned = 0;
    uint32_t total_possible_points = 0;

    for(TestCase const & test : tests) {
        BufferedPrinter printer;
        FileInputter inputter;

        verify_count = 0;
        verify_valid = 0;

        total_possible_points += test.points;

        try {
            simInit(printer, inputter);
        } catch(utils::exception const & e) {
            std::cout << e.what();
            return 1;
        }

        std::cout << "Test Case: " << test.name;
        if(test.randomize) {
            // TODO: randomize machine
            std::cout << " (Randomized Machine)";
        }
        std::cout << std::endl;


        try {
            test.test_func();
        } catch(utils::exception const & e) { }

        float percent_points_earned = ((float) verify_valid) / verify_count;
        uint32_t points_earned = (uint32_t) ( percent_points_earned * test.points);
        std::cout << "test points earned: " << points_earned << "/" << test.points << " ("
                  << (percent_points_earned * 100) << "%)\n";
        std::cout << "==========\n";

        total_points_earned += points_earned;

        simShutdown();
    }

    std::cout << "==========\n";
    float percent_points_earned = ((float) total_points_earned) / total_possible_points;
    std::cout << "total points earned: " << total_points_earned << "/" << total_possible_points << " ("
              << (percent_points_earned * 100) << "%)\n";

    return 0;
}
