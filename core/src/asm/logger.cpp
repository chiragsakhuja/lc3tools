#include <cstdarg>
#include <iostream>
#include <string>

#include "logger.h"
#include "tokens.h"

void AssemblerLogger::printf(int level, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    printer.vprintf(level, format, args);
    va_end(args);
}

void AssemblerLogger::printfMessage(int level, std::string const & filename, Token const * tok, 
    std::string const & line, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    vxprintfMessage(level, filename, tok->col_num, tok->length, tok, line, format, args);
    va_end(args);
}

void AssemblerLogger::xprintfMessage(int level, std::string const & filename, int col_num,
    int length, Token * const tok, std::string const & line, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    vxprintfMessage(level, filename, col_num, length, tok, line, format, args);
    va_end(args);
}

void AssemblerLogger::vxprintfMessage(int level, std::string const & filename, int col_num, int length,
    Token const * tok, std::string const & line, char const * format, va_list args) const
{
    printer.setColor(utils::PrintColor::PRINT_COLOR_BOLD);
    printer.print(printer.toString("%s:%d:%d: ", filename.c_str(), tok->row_num + 1, col_num + 1));

    printer.vprintf(level, format, args);
    printer.print(printer.toString("%s\n", line.c_str()));

    printer.setColor(utils::PrintColor::PRINT_COLOR_BOLD);
    printer.setColor(utils::PrintColor::PRINT_COLOR_GREEN);

    for(int i = 0; i < col_num; i++) {
        printer.print(" ");
    }
    printer.print("^");

    for(int i = 0; i < length - 1; i++) {
        printer.print("~");
    }

    printer.setColor(utils::PrintColor::PRINT_COLOR_RESET);
    printer.print("\n");
}

void AssemblerLogger::newline(void) const
{
    printer.print("\n");
}
