#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

class AssemblerPrinter : public Printer
{
public:
    static AssemblerPrinter& getInstance();

    static void printAssemblyError(const std::string& filename, const Token *tok, const std::string& line, const char *format, ...);
    static void printAssemblyErrorX(const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, ...);
    static void printAssemblyWarning(const std::string& filename, const Token *tok, const std::string& line, const char *format, ...);
    static void printAssemblyWarningX(const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, ...);

private:
    AssemblerPrinter() {}

    static void printAssemblyErrorXVL(const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, va_list args);
    static void printAssemblyWarningXVL(const std::string& filename, int colNum, int length, const Token *tok, const std::string& line, const char *format, va_list args);

    AssemblerPrinter(AssemblerPrinter const&) = delete;
    void operator=(AssemblerPrinter const&)   = delete;
};

#endif
