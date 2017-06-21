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

    template<typename ... Args>
    void printf(int level, bool bold, std::string const & format, Args ... args) const;
    template<typename ... Args>
    void printfMessage(int level, std::string const & filename, Token const * tok, std::string const & line,
        std::string const & format, Args ... args) const;
    template<typename ... Args>
    void xprintfMessage(int level, std::string const & filename, int col_num, int length, Token const * tok,
        std::string const & line, std::string const & format, Args ... args) const;
    void newline(void) const { printer.print("\n"); }
};

template<typename ... Args>
void AssemblerLogger::printf(int level, bool bold, std::string const & format, Args ... args) const
{
    printer.printf(level, bold, format, args...);
}

template<typename ... Args>
void AssemblerLogger::printfMessage(int level, std::string const & filename, Token const * tok, 
    std::string const & line, std::string const & format, Args ... args) const
{
    xprintfMessage(level, filename, tok->col_num, tok->length, tok, line, format, args...);
}

template<typename ... Args>
void AssemblerLogger::xprintfMessage(int level, std::string const & filename, int col_num,
    int length, Token const * tok, std::string const & line, std::string const & format, Args ... args) const
{
    printer.setColor(utils::PrintColor::PRINT_COLOR_BOLD);
    printer.print(printer.toString("%s:%d:%d: ", filename.c_str(), tok->row_num + 1, col_num + 1));
 
    printer.printf(level, true, format, args...);
    printer.print(printer.toString("%s", line.c_str()));
    printer.newline();
 
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
    printer.newline();
}

#endif
