#include <iostream>
#include <string>
#include <cstdarg>

#include "tokens.h"
#include "logger.h"

void AssemblerLogger::printfMessage(int level, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    printer.vprintfMessage(level, format, args);
    va_end(args);
}

void AssemblerLogger::printfAssemblyMessage(int level, std::string const & filename, Token const * tok, std::string const & line, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    vxprintfAssemblyMessage(level, filename, tok->col_num, tok->length, tok, line, format, args);
    va_end(args);
}

void AssemblerLogger::xprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token * const tok, std::string const & line, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    vxprintfAssemblyMessage(level, filename, col_num, length, tok, line, format, args);
    va_end(args);
}

void AssemblerLogger::vxprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token const * tok, std::string const & line, char const * format, va_list args) const
{
    printer.setColor(utils::PrintColor::PRINT_COLOR_BOLD);
    printf("%s:%d:%d: ", filename.c_str(), tok->row_num + 1, col_num + 1);

    printer.vprintfMessage(level, format, args);
    printf("%s\n", line.c_str());

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
    printer.print("\n\n");
}
