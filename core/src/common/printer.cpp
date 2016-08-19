#include <cstdarg>
#include <string>

#include "printer.h"

void utils::Printer::printf(int type, char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(type, format, args);
    va_end(args);
}

void utils::Printer::vprintf(int type, char const * format, va_list args)
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

            case EXTRA:
                color = PRINT_COLOR_BLUE;
                label = "extra";
                break;

            default: break;
        }

        vxprintf(type, color, label.c_str(), format, args);
        print("\n");
    }
}

void utils::Printer::vxprintf(int level, int color, char const * label, char const * format, va_list args)
{
    setColor(PRINT_COLOR_BOLD);
    setColor(color);
    print(toString("%s: ", label));
    setColor(PRINT_COLOR_RESET);

    setColor(PRINT_COLOR_BOLD);
    print(vtoString(format, args));
    setColor(PRINT_COLOR_RESET);
}

std::string utils::Printer::toString(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    std::string ret(vtoString(format, args));
    va_end(args);
    return ret;
}

std::string utils::Printer::vtoString(char const * format, va_list args)
{
    va_list copy;
    va_copy(copy, args);

    int len = std::vsnprintf(nullptr, 0, format, args);
    char * str = new char[len + 1];

    va_copy(args, copy);
    std::vsnprintf(str, len + 1, format, args);

    std::string ret(str);
    delete[] str;
    return ret;
}
