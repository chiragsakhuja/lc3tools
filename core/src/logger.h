#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

#include "utils.h"

namespace core {
    enum PrintType {
          PRINT_TYPE_FATAL_ERROR = 0
        , PRINT_TYPE_ERROR
        , PRINT_TYPE_WARNING
        , PRINT_TYPE_NOTE
        , PRINT_TYPE_INFO
        , PRINT_TYPE_DEBUG
        , PRINT_TYPE_EXTRA
    };

    class Logger
    {
    protected:
        utils::IPrinter & printer;
    public:
        Logger(utils::IPrinter & printer) : printer(printer) {}

        template<typename ... Args>
        void printf(int level, bool bold, std::string const & format, Args ... args) const;
        void newline(void) const { printer.newline(); }
        void print(std::string const & str) { printer.print(str); }
    };

    class AssemblerLogger : public Logger
    {
    public:
        using Logger::Logger;

        template<typename ... Args>
        void printfMessage(int level, std::string const & filename, Token const * tok, std::string const & line,
            std::string const & format, Args ... args) const;
        template<typename ... Args>
        void xprintfMessage(int level, std::string const & filename, int col_num, int length, Token const * tok,
            std::string const & line, std::string const & format, Args ... args) const;
    };
}

template<typename ... Args>
void core::Logger::printf(int type, bool bold, std::string const & format, Args ... args) const
{
    int color = utils::PRINT_COLOR_RESET;
    std::string label = "";

    if(type <= _PRINT_LEVEL) {
        switch(type) {
            case PRINT_TYPE_ERROR:
                color = utils::PRINT_COLOR_RED;
                label = "error";
                break;

            case PRINT_TYPE_WARNING:
                color = utils::PRINT_COLOR_YELLOW;
                label = "warning";
                break;

            case PRINT_TYPE_NOTE:
                color = utils::PRINT_COLOR_GRAY;
                label = "note";
                break;

            case PRINT_TYPE_INFO:
                color = utils::PRINT_COLOR_GREEN;
                label = "info";
                break;

            case PRINT_TYPE_DEBUG:
                color = utils::PRINT_COLOR_MAGENTA;
                label = "debug";
                break;

            case PRINT_TYPE_EXTRA:
                color = utils::PRINT_COLOR_BLUE;
                label = "extra";
                break;

            default: break;
        }

        printer.setColor(utils::PRINT_COLOR_BOLD);
        printer.setColor(color);
        printer.print(ssprintf("%s: ", label.c_str()));
        printer.setColor(utils::PRINT_COLOR_RESET);

        if(bold) {
            printer.setColor(utils::PRINT_COLOR_BOLD);
        }

        printer.print(ssprintf(format, args...));
        printer.setColor(utils::PRINT_COLOR_RESET);

        printer.newline();
    }
}

template<typename ... Args>
void core::AssemblerLogger::printfMessage(int level, std::string const & filename, Token const * tok, 
    std::string const & line, std::string const & format, Args ... args) const
{
    xprintfMessage(level, filename, tok->col_num, tok->length, tok, line, format, args...);
}

template<typename ... Args>
void core::AssemblerLogger::xprintfMessage(int level, std::string const & filename, int col_num,
    int length, Token const * tok, std::string const & line, std::string const & format, Args ... args) const
{
    printer.setColor(utils::PRINT_COLOR_BOLD);
    printer.print(ssprintf("%s:%d:%d: ", filename.c_str(), tok->row_num + 1, col_num + 1));
 
    printf(level, true, format, args...);
    printer.print(ssprintf("%s", line.c_str()));
    printer.newline();
 
    printer.setColor(utils::PRINT_COLOR_BOLD);
    printer.setColor(utils::PRINT_COLOR_GREEN);
 
    for(int i = 0; i < col_num; i++) {
        printer.print(" ");
    }
    printer.print("^");
 
    for(int i = 0; i < length - 1; i++) {
        printer.print("~");
    }
 
    printer.setColor(utils::PRINT_COLOR_RESET);
    printer.newline();
}

#endif
