/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef FILE_PRINTER_H
#define FILE_PRINTER_H

#include <iostream>
#include <fstream>

#include "printer.h"

namespace lc3
{
    class FilePrinter : public utils::IPrinter
    {
    public:
        FilePrinter(std::string const & filename) : output(filename) { }

        virtual void setColor(utils::PrintColor color) override
        {
            (void) color;
            return;
        }

        virtual void print(std::string const & string) override
        {
            output << string << std::flush;
        }

        virtual void newline(void) override
        {
            output << "\n";
        }

    private:
        std::ofstream output;
    };
};

#endif
