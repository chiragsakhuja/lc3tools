#ifndef PRINTER_H
#define PRINTER_H

namespace utils
{
    enum PrintColor {
          PRINT_COLOR_RED
        , PRINT_COLOR_YELLOW
        , PRINT_COLOR_GREEN
        , PRINT_COLOR_MAGENTA
        , PRINT_COLOR_BLUE
        , PRINT_COLOR_GRAY
        , PRINT_COLOR_BOLD
        , PRINT_COLOR_RESET
    };

    class IPrinter
    {
    public:
        IPrinter(void) = default;
        virtual ~IPrinter(void) = default;

        virtual void setColor(int color) = 0;
        virtual void print(std::string const & string) = 0;
        virtual void newline(void) = 0;
    };
};

#endif
