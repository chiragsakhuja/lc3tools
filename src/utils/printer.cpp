#include <iostream>
#include <cstdarg>

#include "printer.h"

const char *Printer::RED     = "\033[31m";
const char *Printer::YELLOW  = "\033[33m";
const char *Printer::GREEN   = "\033[32m";
const char *Printer::MAGENTA = "\033[35m";
const char *Printer::RESET   = "\033[0m";
const char *Printer::BOLD    = "\033[1m";

Printer& Printer::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static Printer instance;

    return instance;
}

void Printer::printError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printErrorVL(format, args);
    va_end(args);
}

void Printer::printWarning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printWarningVL(format, args);
    va_end(args);
}

void Printer::printInfo(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printInfoVL(format, args);
    va_end(args);
}

void Printer::printDebugInfo(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printDebugInfoVL(format, args);
    va_end(args);
}

void Printer::printErrorVL(const char *format, va_list args)
{
    std::cout << BOLD << RED << "error: " << RESET << BOLD;
    std::vprintf(format, args);
    std::cout << RESET << std::endl;
}

void Printer::printWarningVL(const char *format, va_list args)
{
    std::cout << BOLD << YELLOW << "warning: " << RESET << BOLD;
    std::vprintf(format, args);
    std::cout << RESET << std::endl;
}

void Printer::printInfoVL(const char *format, va_list args)
{
    std::cout << BOLD << GREEN << "info: " << RESET << BOLD;
    std::vprintf(format, args);
    std::cout << RESET << std::endl;
}

void Printer::printDebugInfoVL(const char *format, va_list args)
{
#ifdef DEBUG
    std::cout << BOLD << MAGENTA << "debug: " << RESET << BOLD;
    std::vprintf(format, args);
    std::cout << RESET << std::endl;
#endif
}

void Printer::printNChars(std::ostream &stream, char c, int count)
{
    for(int i = 0; i < count; i++) {
        stream << c;
    }
}
