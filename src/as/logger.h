#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

#include "tokens.h"
#include "../utils/printer.h"

class AssemblerLogger
{
private:
    const utils::Printer & printer;

public:
    AssemblerLogger(utils::Printer const & printer) : printer(printer) {}
    AssemblerLogger(AssemblerLogger const &) = default;
    AssemblerLogger & operator=(AssemblerLogger const &) = default;

    void printfMessage(int level, char const * format, ...) const;
    void printfAssemblyMessage(int level, std::string const & filename, Token const * tok, std::string const & line, char const * format, ...) const;
    void xprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token * const tok, std::string const & line, char const * format, ...) const;

private:
    void vxprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token const * tok, std::string const & line, char const * format, va_list args) const;
};

#endif
