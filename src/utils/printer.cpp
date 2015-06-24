#include <iostream>
#include <cstdarg>

#include "printer.h"

Printer& Printer::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static Printer instance;

    return instance;
}

void Printer::printfMessage(int type, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintfMessage(type, format, args);
    print("\n");
    va_end(args);
}

void Printer::vprintfMessage(int type, const char *format, va_list args)
{
    int level = type;
    int color = PRINT_MODE_RESET;
    std::string label = "";

    if(level <= _PRINT_LEVEL) {
        switch(type) {
            case ERROR:
                color = PRINT_MODE_RED;
                label = "error";
                break;

            case WARNING:
                color = PRINT_MODE_YELLOW;
                label = "warning";
                break;

            case INFO:
                color = PRINT_MODE_GREEN;
                label = "info";
                break;

            case DEBUG:
                color = PRINT_MODE_MAGENTA;
                label = "debug";
                break;

            default: break;
        }

        vxprintfMessage(level, color, label.c_str(), format, args);
    }
}

void Printer::vxprintfMessage(int level, int color, const char *label, const char *format, va_list args)
{
    setMode(PRINT_MODE_BOLD);
    setMode(color);
    printf("%s: ", label);
    setMode(PRINT_MODE_RESET);

    setMode(PRINT_MODE_BOLD);
    vprintf(format, args);
    setMode(PRINT_MODE_RESET);
}

void Printer::printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void Printer::vprintf(const char *format, va_list args)
{
    va_list copy;
    va_copy(copy, args);

    int len = std::vsnprintf(nullptr, 0, format, args);
    char *string = new char[len + 1];

    va_copy(args, copy);
    std::vsnprintf(string, len + 1, format, args);
    print(string);

    delete[] string;
}

void Printer::setMode(int mode)
{
    switch(mode) {
        case PRINT_MODE_RED    : std::cout << "\033[31m"; break;
        case PRINT_MODE_YELLOW : std::cout << "\033[33m"; break;
        case PRINT_MODE_GREEN  : std::cout << "\033[32m"; break;
        case PRINT_MODE_MAGENTA: std::cout << "\033[35m"; break;
        case PRINT_MODE_BOLD   : std::cout << "\033[1m" ; break;
        case PRINT_MODE_RESET  : std::cout << "\033[0m" ; break;
        default                :                          break;
    }
}

void Printer::print(const char *string)
{
    std::cout << string;
}
