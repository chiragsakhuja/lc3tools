#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

class AssemblerPrinter : public Printer
{
public:
    static AssemblerPrinter& getInstance();

    static void printfAssemblyMessage(int level, const std::string& filename, const Token *tok, const std::string& line, const char *format, ...);
    static void xprintfAssemblyMessage(int level, const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, ...);

private:
    AssemblerPrinter() {}

    static void vxprintfAssemblyMessage(int level, const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, va_list args);

    AssemblerPrinter(AssemblerPrinter const&) = delete;
    void operator=(AssemblerPrinter const&)   = delete;
};

#endif
