/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
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

    enum class PrintType {
          P_NONE = 0
        , P_SIM_OUTPUT
        , P_FATAL_ERROR
        , P_ERROR
        , P_WARNING
        , P_NOTE
        , P_INFO
        , P_DEBUG
        , P_EXTRA
        , P_SPAM
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
};
};

#endif
