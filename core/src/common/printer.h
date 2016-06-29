#ifndef PRINTER_H
#define PRINTER_H

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 2
#endif

#include <cstdio>
#include <string>

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
        Printer(void) = default;
        Printer(Printer const &) = default;
        Printer & operator=(Printer const &) = default;
        virtual ~Printer(void) = default;

        void printf(int type, char const * format, ...);
        void vprintf(int type, char const * format, va_list args);
        std::string toString(char const * format, ...);
        std::string vtoString(char const * format, va_list args);

        virtual void setColor(int color) = 0;
        virtual void print(std::string const & string) = 0;

    protected:
        void vxprintf(int level, int color, char const * label, char const * format, va_list args);
    };
};


#endif
