#ifndef CONSOLE_PRINTER_H
#define CONSOLE_PRINTER_H

#include <string>
#include "printer.h"

namespace utils
{
    class ConsolePrinter : public Printer
    {
    public:
        virtual void setColor(int color) override;
        virtual void print(std::string const & string) override;
        virtual void newline(void) override;
    };
};

#endif
