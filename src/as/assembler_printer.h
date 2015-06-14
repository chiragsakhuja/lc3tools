#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

class AssemblerPrinter : public Printer
{
public:
    static AssemblerPrinter& getInstance();

    static void printAssemblyError(const char *filename, Token *tok, const char *line, const char *format, ...);

private:
    AssemblerPrinter() {}

    AssemblerPrinter(AssemblerPrinter const&) = delete;
    void operator=(AssemblerPrinter const&)  = delete;
};

#endif
