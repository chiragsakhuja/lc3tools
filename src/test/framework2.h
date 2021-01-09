/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cstdint>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "framework_common.h"

namespace framework2
{
class Tester;

using test_func_t = std::function<void(lc3::sim &, Tester &, double total_points)>;

extern std::function<void(Tester &)> setup;
extern std::function<void(void)> shutdown;
extern std::function<void(lc3::sim &)> testBringup;
extern std::function<void(lc3::sim &)> testTeardown;

struct TestCase
{
    std::string name;
    test_func_t test_func;
    double points;
    bool randomize;

    TestCase(std::string const & name, test_func_t test_func, double points, bool randomize);
};

class Tester
{
private:
    std::vector<TestCase> tests;
    bool print_output, ignore_privilege, verbose;
    uint32_t print_level;
    uint64_t seed;
    std::vector<std::string> obj_filenames;
    lc3::core::SymbolTable symbol_table;

    BufferedPrinter * printer;
    StringInputter * inputter;
    lc3::sim * simulator;

    double test_points_earned;

    std::pair<double, double> testAll(void);
    std::pair<double, double> testSingle(std::string const & test_name);

    std::pair<double, double> testSingle(TestCase const & test);
    void resetTestPoints(void);

    double checkSimilarityHelper(std::vector<char> const & source, std::vector<char> const & target) const;

    friend int main(int argc, char * argv[]);

public:
    enum PreprocessType {
        IgnoreCase = 1,
        IgnoreWhitespace = 2,
        IgnorePunctuation = 4
    };

    Tester(bool print_output, uint32_t print_level, bool ignore_privilege, bool verbose,
        uint64_t seed, std::vector<std::string> const & obj_filenames);

    void registerTest(std::string const & name, test_func_t test_func, double points, bool randomize);
    void verify(std::string const & label, bool pred, double points);

    void output(std::string const & message);
    void error(std::string const & label, std::string const & message);

    void setInputString(std::string const & source) { inputter->setString(source); }
    void setInputCharDelay(uint32_t inst_count) { inputter->setCharDelay(inst_count); }

    std::string getOutput(void) const;
    void clearOutput(void) { printer->clear(); }
    bool checkMatch(std::string const & a, std::string const & b) const { return a == b; }
    bool checkContain(std::string const & str, std::string const & expected_part) const;
    double checkSimilarity(std::string const & source, std::string const & target) const;
    std::string getPreprocessedString(std::string const & str, uint64_t type) const;

    void setSymbolTable(lc3::core::SymbolTable const & symbol_table) { this->symbol_table = symbol_table; }
    lc3::core::SymbolTable const & getSymbolTable(void) { return symbol_table; }
};

    int main(int argc, char * argv[]);
};
