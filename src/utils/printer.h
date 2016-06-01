#ifndef PRINTER_H
#define PRINTER_H

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 2
#endif

class Printer
{
public:
    enum PrintType {
          FATAL_ERROR = 0
        , ERROR
        , WARNING
        , INFO
        , DEBUG
    };

    void printfMessage(int type, const char * format, ...) const;

    Printer() = default;
    Printer(Printer const &) = default;
    Printer & operator=(Printer const &) = default;

protected:
    enum PrintMode {
          PRINT_MODE_RED
        , PRINT_MODE_YELLOW
        , PRINT_MODE_GREEN
        , PRINT_MODE_MAGENTA
        , PRINT_MODE_BOLD
        , PRINT_MODE_RESET
    };

    void vprintfMessage(int type, const char * format, va_list args) const;
    void vxprintfMessage(int level, int color, const char * label, const char * format, va_list args) const;
    void printf(const char * format, ...) const;
    void vprintf(const char * format, va_list args) const;
    void setMode(int mode) const;
    virtual void print(const char * string) const;
};

#endif
