#ifndef PRINTER_H
#define PRINTER_H

#include <string>

namespace lc3
{
namespace utils
{
    enum class PrintColor {
          RED = 0
        , YELLOW
        , GREEN
        , MAGENTA
        , BLUE
        , GRAY
        , BOLD
        , RESET
    };

    class IPrinter
    {
    public:
        IPrinter(void) = default;
        virtual ~IPrinter(void) = default;

        virtual void setColor(PrintColor color) = 0;
        virtual void print(std::string const & string) = 0;
        virtual void newline(void) = 0;
    };

    class NullPrinter : public IPrinter
    {
    public:
        virtual void setColor(PrintColor) override {}
        virtual void print(std::string const &) override {}
        virtual void newline(void) override {}
    };
};
};

#endif
