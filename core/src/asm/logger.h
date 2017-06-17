#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

#include "tokens.h"
#include "../common/printer.h"

class AssemblerLogger
{
private:
    utils::Printer & printer;

public:
    AssemblerLogger(utils::Printer & printer) : printer(printer) {}
    AssemblerLogger(AssemblerLogger const &) = default;
    AssemblerLogger & operator=(AssemblerLogger const &) = default;

    void printf(int level, bool bold, char const * format, ...) const;
    void printfMessage(int level, std::string const & filename, Token const * tok, std::string const & line,
        char const * format, ...) const;
    void xprintfMessage(int level, std::string const & filename, int col_num, int length, Token * const tok,
        std::string const & line, char const * format, ...) const;
    void newline(void) const;

private:
    void vxprintfMessage(int level, std::string const & filename, int col_num, int length, Token const * tok,
        std::string const & line, char const * format, va_list args) const;
};

#endif
