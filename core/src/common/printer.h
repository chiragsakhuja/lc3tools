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
        , PRINT_COLOR_BLUE
        , PRINT_COLOR_GRAY
        , PRINT_COLOR_BOLD
        , PRINT_COLOR_RESET
    };

    enum PrintType {
          PRINT_TYPE_FATAL_ERROR = 0
        , PRINT_TYPE_ERROR
        , PRINT_TYPE_WARNING
        , PRINT_TYPE_NOTE
        , PRINT_TYPE_INFO
        , PRINT_TYPE_DEBUG
        , PRINT_TYPE_EXTRA
    };

    class Printer
    {
    public:
        Printer(void) = default;
        Printer(Printer const &) = default;
        Printer & operator=(Printer const &) = default;
        virtual ~Printer(void) = default;

        template<typename ... Args>
        void printf(int type, bool bold, std::string const & str, Args ... args);
        template<typename ... Args>
        std::string toString(std::string const & str, Args ... args);

        virtual void setColor(int color) = 0;
        virtual void print(std::string const & string) = 0;
        virtual void newline(void) = 0;
    };
};

template<typename ... Args>
void utils::Printer::printf(int type, bool bold, std::string const & format, Args ... args)
{
    int color = PRINT_COLOR_RESET;
    std::string label = "";

    if(type <= _PRINT_LEVEL) {
        switch(type) {
            case PRINT_TYPE_ERROR:
                color = PRINT_COLOR_RED;
                label = "error";
                break;

            case PRINT_TYPE_WARNING:
                color = PRINT_COLOR_YELLOW;
                label = "warning";
                break;

            case PRINT_TYPE_NOTE:
                color = PRINT_COLOR_GRAY;
                label = "note";
                break;

            case PRINT_TYPE_INFO:
                color = PRINT_COLOR_GREEN;
                label = "info";
                break;

            case PRINT_TYPE_DEBUG:
                color = PRINT_COLOR_MAGENTA;
                label = "debug";
                break;

            case PRINT_TYPE_EXTRA:
                color = PRINT_COLOR_BLUE;
                label = "extra";
                break;

            default: break;
        }

        setColor(PRINT_COLOR_BOLD);
        setColor(color);
        print(toString("%s: ", label.c_str()));
        setColor(PRINT_COLOR_RESET);

        if(bold) {
            setColor(PRINT_COLOR_BOLD);
        }

        print(toString(format, args...));
        setColor(PRINT_COLOR_RESET);

        newline();
    }
}

template<typename ... Args>
std::string utils::Printer::toString(std::string const & format, Args ... args)
{
    int len = std::snprintf(nullptr, 0, format.c_str(), args...);
    char * str = new char[len + 1];

    std::snprintf(str, len + 1, format.c_str(), args...);

    std::string ret(str);
    delete[] str;
    return ret;
}

#endif
