#ifndef CONSOLE_PRINTER_H
#define CONSOLE_PRINTER_H

#include "printer.h"

namespace lc3
{
    class ConsolePrinter : public utils::IPrinter
    {
    public:
        virtual void setColor(utils::PrintColor color) override;
        virtual void print(std::string const & string) override;
        virtual void newline(void) override;
    };
};

#endif
