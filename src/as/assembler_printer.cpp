#include <iostream>
#include <string>
#include <cstdarg>

#include "tokens.h"
#include "utils/printer.h"
#include "assembler_printer.h"

void AssemblerPrinter::printfAssemblyMessage(int level, std::string const & filename, Token const * tok, std::string const & line, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    vxprintfAssemblyMessage(level, filename, tok->col_num, tok->length, tok, line, format, args);
    va_end(args);
}

void AssemblerPrinter::xprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token * const tok, std::string const & line, char const * format, ...) const
{
    va_list args;
    va_start(args, format);
    vxprintfAssemblyMessage(level, filename, col_num, length, tok, line, format, args);
    va_end(args);
}

void AssemblerPrinter::vxprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token const * tok, std::string const & line, char const * format, va_list args) const
{
    setMode(PRINT_MODE_BOLD);
    printf("%s:%d:%d: ", filename.c_str(), tok->row_num + 1, col_num + 1);

    vprintfMessage(level, format, args);
    printf("%s\n", line.c_str());

    setMode(PRINT_MODE_BOLD);
    setMode(PRINT_MODE_GREEN);

    for(int i = 0; i < col_num; i++) {
        print(" ");
    }
    print("^");

    for(int i = 0; i < length - 1; i++) {
        print("~");
    }

    setMode(PRINT_MODE_RESET);
    print("\n\n");
}
