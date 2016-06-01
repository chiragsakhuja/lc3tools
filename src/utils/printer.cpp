#include <cstdarg>
#include <string>

#include "printer.h"

void utils::Printer::printfMessage(int type, const char * format, ...) const
{
    va_list args;
    va_start(args, format);
    vprintfMessage(type, format, args);
    va_end(args);
}

void utils::Printer::vprintfMessage(int type, const char * format, va_list args) const
{
    int color = PRINT_COLOR_RESET;
    std::string label = "";

    if(type <= _PRINT_LEVEL) {
        switch(type) {
            case ERROR:
                color = PRINT_COLOR_RED;
                label = "error";
                break;

            case WARNING:
                color = PRINT_COLOR_YELLOW;
                label = "warning";
                break;

            case INFO:
                color = PRINT_COLOR_GREEN;
                label = "info";
                break;

            case DEBUG:
                color = PRINT_COLOR_MAGENTA;
                label = "debug";
                break;

            default: break;
        }

        vxprintfMessage(type, color, label.c_str(), format, args);
        print("\n");
    }
}

void utils::Printer::vxprintfMessage(int level, int color, const char * label, const char * format, va_list args) const
{
    setColor(PRINT_COLOR_BOLD);
    setColor(color);
    printf("%s: ", label);
    setColor(PRINT_COLOR_RESET);

    setColor(PRINT_COLOR_BOLD);
    vprintf(format, args);
    setColor(PRINT_COLOR_RESET);
}

void utils::Printer::printf(const char * format, ...) const
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void utils::Printer::vprintf(const char * format, va_list args) const
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
