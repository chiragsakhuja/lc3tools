#ifndef LOGGER_H
#define LOGGER_H

#include <vector>

#include "printer.h"
#include "tokens.h"
#include "utils.h"

namespace lc3::utils
{
    enum class PrintType {
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
        lc3::utils::IPrinter & printer;
        bool log_enable;
        uint32_t log_level;

    public:
        Logger(bool log_enable, utils::IPrinter & printer) : printer(printer), log_enable(log_enable) {}

        template<typename ... Args>
        void printf(PrintType level, bool bold, std::string const & format, Args ... args) const;
        void newline(void) const { printer.newline(); }
        void print(std::string const & str) { printer.print(str); }
    };

    class AssemblerLogger : public Logger
    {
    public:
        using Logger::Logger;

        template<typename ... Args>
        void printfMessage(PrintType level, Token const * tok, std::string const & format, Args ... args) const;
        template<typename ... Args>
        void xprintfMessage(PrintType level, int col_num, int length, Token const * tok, std::string const & format,
            Args ... args) const;

        std::string filename;
        std::vector<std::string> asm_blob;
    };
};

template<typename ... Args>
void lc3::utils::Logger::printf(lc3::utils::PrintType type, bool bold, std::string const & format, Args ... args) const
{
    if(! log_enable) { return; }
    lc3::utils::PrintColor color = lc3::utils::PrintColor::PRINT_COLOR_RESET;
    std::string label = "";

    if(static_cast<uint32_t>(type) <= log_level) {
        switch(type) {
            case PrintType::PRINT_TYPE_ERROR:
                color = lc3::utils::PrintColor::PRINT_COLOR_RED;
                label = "error";
                break;

            case PrintType::PRINT_TYPE_WARNING:
                color = lc3::utils::PrintColor::PRINT_COLOR_YELLOW;
                label = "warning";
                break;

            case PrintType::PRINT_TYPE_NOTE:
                color = lc3::utils::PrintColor::PRINT_COLOR_GRAY;
                label = "note";
                break;

            case PrintType::PRINT_TYPE_INFO:
                color = lc3::utils::PrintColor::PRINT_COLOR_GREEN;
                label = "info";
                break;

            case PrintType::PRINT_TYPE_DEBUG:
                color = lc3::utils::PrintColor::PRINT_COLOR_MAGENTA;
                label = "debug";
                break;

            case PrintType::PRINT_TYPE_EXTRA:
                color = lc3::utils::PrintColor::PRINT_COLOR_BLUE;
                label = "extra";
                break;

            default: break;
        }

        printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_BOLD);
        printer.setColor(color);
        printer.print(lc3::utils::ssprintf("%s: ", label.c_str()));
        printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_RESET);

        if(bold) {
            printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_BOLD);
        }

        printer.print(lc3::utils::ssprintf(format, args...));
        printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_RESET);

        printer.newline();
    }
}

template<typename ... Args>
void lc3::utils::AssemblerLogger::printfMessage(lc3::utils::PrintType level, Token const * tok,
    std::string const & format, Args ... args) const
{
    xprintfMessage(level, tok->col_num, tok->length, tok, format, args...);
}

template<typename ... Args>
void lc3::utils::AssemblerLogger::xprintfMessage(lc3::utils::PrintType level, int col_num, int length, Token const * tok,
    std::string const & format, Args ... args) const
{
    if(! log_enable) { return; }
    printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_BOLD);
    printer.print(lc3::utils::ssprintf("%s:%d:%d: ", filename.c_str(), tok->row_num + 1, col_num + 1));
 
    printf(level, true, format, args...);
    printer.print(lc3::utils::ssprintf("%s", asm_blob[tok->row_num].c_str()));
    printer.newline();
 
    printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_BOLD);
    printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_GREEN);
 
    for(int i = 0; i < col_num; i++) {
        printer.print(" ");
    }
    printer.print("^");
 
    for(int i = 0; i < length - 1; i++) {
        printer.print("~");
    }
 
    printer.setColor(lc3::utils::PrintColor::PRINT_COLOR_RESET);
    printer.newline();
}

#endif
