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
        Printer(void) = default;
        Printer(Printer const &) = default;
        Printer & operator=(Printer const &) = default;
        virtual ~Printer(void) = default;

        void printf(int type, char const * format, ...) const;
        void vprintf(int type, char const * format, va_list args) const;
        std::string toString(char const * format, ...) const;
        std::string vtoString(char const * format, va_list args) const;

        virtual void setColor(int color) const = 0;
        virtual void print(std::string const & string) const = 0;

    protected:
        void vxprintf(int level, int color, char const * label, char const * format, va_list args) const;
    };
};


#endif
