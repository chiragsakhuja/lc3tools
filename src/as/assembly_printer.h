#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

#include "../utils/printer.h"

class AssemblyPrinter : public Printer
{
public:
    static AssemblyPrinter& getInstance();

    static void printAssemblyError(const char *filename, Token *tok, const char *line, const char *format, ...);

private:
    AssemblyPrinter() {}

    AssemblyPrinter(AssemblyPrinter const&) = delete;
    void operator=(AssemblyPrinter const&)  = delete;
};

#endif
