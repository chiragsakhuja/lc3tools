#include <iostream>
#include <string>
#include <cstdarg>

#include "tokens.h"
#include "assembly_printer.h"

AssemblyPrinter& AssemblyPrinter::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static AssemblyPrinter instance;

    return instance;
}

void AssemblyPrinter::printAssemblyError(const char *filename, Token *tok, const char *line, const char *format, ...)
{
    std::cerr << BOLD << filename << ":" << tok->rowNum + 1 << ":" << tok->colNum + 1 << ": " << RED << "error: " << RESET << BOLD;

    va_list args;
    va_start(args, format);
    printErrorVL(format, args);
    va_end(args);
    std::cerr << RESET << std::endl;

    std::cerr << line << std::endl;

    printNChars(std::cerr, ' ', tok->colNum);
    std::cerr << BOLD << GREEN << "^";
    printNChars(std::cerr, '~', tok->length - 1);
    std::cerr << RESET << std::endl << std::endl;
}
