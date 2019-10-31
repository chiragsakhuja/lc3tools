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

#include "inputter.h"
#include "interface.h"
#include "printer.h"

struct TestCase;

extern std::vector<TestCase> tests;
extern uint32_t verify_count;
extern uint32_t verify_valid;

class BufferedPrinter : public lc3::utils::IPrinter
{
public:
    BufferedPrinter(bool print_output) : print_output(print_output) {}

    std::vector<char> display_buffer;

    virtual void setColor(lc3::utils::PrintColor color) override { (void) color; }
    virtual void print(std::string const & string) override;
    virtual void newline(void) override;
    void clear(void) { display_buffer.clear(); }

private:
    bool print_output;
};

class StringInputter : public lc3::utils::IInputter
{
public:
    StringInputter(void) { setString(""); }
    StringInputter(std::string const & source) { setString(source); }

    void setString(std::string const & source) { setStringAfter(source, 0); }
    void setStringAfter(std::string const & source, uint32_t inst_count);
    void beginInput(void) {}
    bool getChar(char & c);
    void endInput(void) {}

private:
    std::string source;
    uint32_t pos;
    uint32_t inst_delay;
};

class Grader;

using test_func_t = std::function<void(lc3::sim &, Grader &, double total_points)>;

struct TestCase
{
    std::string name;
    test_func_t test_func;
    double points;
    bool randomize;

    TestCase(std::string const & name, test_func_t test_func, double points, bool randomize);
};

class Grader
{
private:
    std::vector<TestCase> tests;
    bool print_output, ignore_privilege, verbose;
    uint32_t print_level;
    std::vector<std::string> obj_filenames;

    double test_points_earned, test_points;

    bool outputCompare(lc3::utils::IPrinter const & printer, std::string check, bool substr);
    std::pair<double, double> grade(TestCase const & test);
    void resetTestPoints(void);

public:
    Grader(bool print_output, uint32_t print_level, bool ignore_privilege, bool verbose,
        std::vector<std::string> const & obj_filenames);

    void registerTest(std::string const & name, test_func_t test_func, double points, bool randomize);
    void verify(std::string const & name, bool pred, double points);
    std::pair<double, double> gradeAll(void);
    std::pair<double, double> grade(std::string const & test_name);
    void output(std::string const & message);
    void error(std::string const & message);
};
