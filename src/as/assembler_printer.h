#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

class AssemblerPrinter : public Printer
{
public:
    AssemblerPrinter() = default;
    AssemblerPrinter(AssemblerPrinter const &) = default;
    AssemblerPrinter & operator=(AssemblerPrinter const &) = default;

    void printfAssemblyMessage(int level, std::string const & filename, Token const * tok, std::string const & line, char const * format, ...) const;
    void xprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token * const tok, std::string const & line, char const * format, ...) const;

private:
    void vxprintfAssemblyMessage(int level, std::string const & filename, int col_num, int length, Token const * tok, std::string const & line, char const * format, va_list args) const;
};

#endif
