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

namespace framework1
{
struct TestCase;

extern std::vector<TestCase> tests;
extern uint32_t verify_count;
extern uint32_t verify_valid;

extern std::function<void(void)> setup;
extern std::function<void(void)> shutdown;
extern std::function<void(lc3::sim &)> testBringup;
extern std::function<void(lc3::sim &)> testTeardown;

struct TestCase
{
    std::string name;
    std::function<void(lc3::sim &, StringInputter &)> test_func;
    uint32_t points;
    bool randomize;

    TestCase(char const * name, std::function<void(lc3::sim &, StringInputter &)> test_func, uint32_t points,
        bool randomize)
    {
        this->name = name;
        this->test_func = test_func;
        this->points = points;
        this->randomize = randomize;
    }
};

bool outputCompare(lc3::utils::IPrinter const & printer, std::string check, bool substr);

int main(int argc, char * argv[]);
};

lc3::core::SymbolTable const & getSymbolTable(void);

#define REGISTER_TEST(name, function, points)                                    \
    framework1::tests.emplace_back( #name , ( function ), ( points ), false);    \
    do {} while(false)
#define REGISTER_RANDOM_TEST(name, function, points)                             \
    framework1::tests.emplace_back( #name , ( function ), ( points ), true);     \
    do {} while(false)
#define VERIFY_NAMED(message, check)                                             \
    framework1::verify_count += 1;                                               \
    std::cout << "  " << ( message ) << " => ";                                  \
    if(( check ) == true) {                                                      \
        framework1::verify_valid += 1;                                           \
        std::cout << "yes\n";                                                    \
    } else {                                                                     \
        std::cout << "no\n";                                                     \
    }                                                                            \
    do {} while(false)
#define VERIFY(check)                                                            \
    VERIFY_NAMED(#check, check)
#define VERIFY_OUTPUT_NAMED(message, check)                                      \
    framework1::verify_count += 1;                                               \
    std::cout << " " << ( message ) << " => ";                                   \
    if(framework1::outputCompare(sim.getPrinter(), check, false)) {              \
        framework1::verify_valid += 1;                                           \
        std::cout << "yes\n";                                                    \
    } else {                                                                     \
        std::cout << "no\n";                                                     \
    }                                                                            \
    static_cast<BufferedPrinter &>(sim.getPrinter()).clear();                    \
    do {} while(false)
#define VERIFY_OUTPUT(check)                                                     \
    VERIFY_OUTPUT_NAMED(#check, check)
#define VERIFY_OUTPUT_HAD_NAMED(message, check)                                  \
    framework1::verify_count += 1;                                               \
    std::cout << " " << ( message ) << " => ";                                   \
    if(framework1::outputCompare(sim.getPrinter(), check, true)) {               \
        framework1::verify_valid += 1;                                           \
        std::cout << "yes\n";                                                    \
    } else {                                                                     \
        std::cout << "no\n";                                                     \
    }                                                                            \
    static_cast<BufferedPrinter &>(sim.getPrinter()).clear();                    \
    do {} while(false)
#define VERIFY_OUTPUT_HAD(check)                                                 \
    VERIFY_OUTPUT_HAD_NAMED(#check, check)
