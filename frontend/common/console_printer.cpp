#include <iostream>

#include "console_printer.h"

void lc3::ConsolePrinter::setColor(lc3::utils::PrintColor color)
{
#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32))
    switch(color) {
        case utils::PrintColor::RED    : std::cout << "\033[31m"          ; break;
        case utils::PrintColor::YELLOW : std::cout << "\033[33m"          ; break;
        case utils::PrintColor::GREEN  : std::cout << "\033[32m"          ; break;
        case utils::PrintColor::MAGENTA: std::cout << "\033[35m"          ; break;
        case utils::PrintColor::BLUE   : std::cout << "\033[34m"          ; break;
        case utils::PrintColor::GRAY   : std::cout << "\033[31;1m\033[30m"; break;
        case utils::PrintColor::BOLD   : std::cout << "\033[1m"           ; break;
        case utils::PrintColor::RESET  : std::cout << "\033[0m"           ; break;
        default                        :                                    break;
    }
#endif
}

void lc3::ConsolePrinter::print(std::string const & string)
{
    std::cout << string << std::flush;
}

void lc3::ConsolePrinter::newline(void)
{
    std::cout << "\n";
}
