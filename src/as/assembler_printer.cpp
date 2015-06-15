#include <iostream>
#include <string>
#include <cstdarg>

#include "tokens.h"
#include "../utils/printer.h"
#include "assembler_printer.h"

AssemblerPrinter& AssemblerPrinter::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static AssemblerPrinter instance;

    return instance;
}

void AssemblerPrinter::printAssemblyError(const std::string& filename, const Token *tok, const std::string& line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printAssemblyErrorXVL(filename, tok->colNum, tok->length, tok, line, format, args);
    va_end(args);
}

void AssemblerPrinter::printAssemblyErrorX(const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printAssemblyErrorXVL(filename, colNum, length, tok, line, format, args);
    va_end(args);
}

void AssemblerPrinter::printAssemblyErrorXVL(const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, va_list args)
{
    std::cerr << BOLD << filename << ":" << tok->rowNum + 1 << ":" << colNum + 1 << ": ";

    printErrorVL(format, args);

    std::cerr << line << std::endl;

    printNChars(std::cerr, ' ', colNum);
    std::cerr << BOLD << GREEN << "^";
    printNChars(std::cerr, '~', length - 1);
    std::cerr << RESET << std::endl << std::endl;
}
