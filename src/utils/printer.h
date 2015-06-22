#ifndef PRINTER_H
#define PRINTER_H

class Printer
{
public:
    static const char *RED;
    static const char *YELLOW;
    static const char *GREEN;
    static const char *MAGENTA;
    static const char *RESET;
    static const char *BOLD;

    static Printer& getInstance();
    static void printError(const char *format, ...);
    static void printWarning(const char *format, ...);
    static void printInfo(const char *format, ...);
    static void printDebugInfo(const char *format, ...);

protected:
    Printer() {}

    static void printNChars(std::ostream &stream, char c, int count);
    static void printErrorVL(const char *format, va_list args);
    static void printWarningVL(const char *format, va_list args);
    static void printInfoVL(const char *format, va_list args);
    static void printDebugInfoVL(const char *format, va_list args);

    // cannot duplicate the singleton
    Printer(Printer const&)        = delete;
    void operator=(Printer const&) = delete;
};

#endif
