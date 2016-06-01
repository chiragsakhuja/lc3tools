#ifndef PRINTER_H
#define PRINTER_H

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 2
#endif

namespace utils
{
    enum PrintColor {
          PRINT_COLOR_RED
        , PRINT_COLOR_YELLOW
        , PRINT_COLOR_GREEN
        , PRINT_COLOR_MAGENTA
        , PRINT_COLOR_BOLD
        , PRINT_COLOR_RESET
    };

    enum PrintType {
          FATAL_ERROR = 0
        , ERROR
        , WARNING
        , INFO
        , DEBUG
    };

    class Printer
    {
    public:

        void printfMessage(int type, const char * format, ...) const;
        void vprintfMessage(int type, const char * format, va_list args) const;

        Printer(void) = default;
        Printer(Printer const &) = default;
        Printer & operator=(Printer const &) = default;
        virtual ~Printer(void) = default;

        virtual void setColor(int color) const = 0;
        virtual void print(const char * string) const = 0;

    protected:
        void vxprintfMessage(int level, int color, const char * label, const char * format, va_list args) const;
        void printf(const char * format, ...) const;
        void vprintf(const char * format, va_list args) const;
    };
};

#endif
