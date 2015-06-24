#include <iostream>
#include <string>
#include <cstdarg>

#include "tokens.h"
#include "utils/printer.h"
#include "assembler_printer.h"

AssemblerPrinter& AssemblerPrinter::getInstance()
{
    // only one copy and guaranteed to be destroyed
    static AssemblerPrinter instance;

    return instance;
}

void AssemblerPrinter::printfAssemblyMessage(int level, const std::string& filename, const Token *tok, const std::string& line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vxprintfAssemblyMessage(level, filename, tok->colNum, tok->length, tok, line, format, args);
    va_end(args);
}

void AssemblerPrinter::xprintfAssemblyMessage(int level, const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vxprintfAssemblyMessage(level, filename, colNum, length, tok, line, format, args);
    va_end(args);
}

void AssemblerPrinter::vxprintfAssemblyMessage(int level, const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, va_list args)
{
    setMode(PRINT_MODE_BOLD);
    printf("%s:%d:%d: ", filename.c_str(), tok->rowNum + 1, colNum + 1);

    vprintfMessage(level, format, args);
    printf("\n%s\n", line.c_str());

    setMode(PRINT_MODE_BOLD);
    setMode(PRINT_MODE_GREEN);

    for(int i = 0; i < colNum; i++) {
        print(" ");
    }
    print("^");

    for(int i = 0; i < length - 1; i++) {
        print("~");
    }

    setMode(PRINT_MODE_RESET);
    print("\n\n");
}
