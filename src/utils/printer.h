#ifndef PRINTER_H
#define PRINTER_H

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 2
#endif

class Printer
{
public:
    enum PrintType {
          ERROR = 0
        , WARNING
        , INFO
        , DEBUG
    };

    static Printer& getInstance();
    static void printfMessage(int type, const char *format, ...);

protected:
    enum PrintMode {
          PRINT_MODE_RED
        , PRINT_MODE_YELLOW
        , PRINT_MODE_GREEN
        , PRINT_MODE_MAGENTA
        , PRINT_MODE_BOLD
        , PRINT_MODE_RESET
    };

    Printer() {}

    static void vprintfMessage(int type, const char *format, va_list args);
    static void vxprintfMessage(int level, int color, const char *label, const char *format, va_list args);
    static void printf(const char *format, ...);
    static void vprintf(const char *format, va_list args);
    static void setMode(int mode);
    static void print(const char *string);

    // cannot duplicate the singleton
    Printer(Printer const&)        = delete;
    void operator=(Printer const&) = delete;
};

#endif
