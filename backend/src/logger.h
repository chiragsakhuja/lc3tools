#ifndef LOGGER_H
#define LOGGER_H

#include <vector>

#include "asm_types.h"
#include "printer.h"
#include "utils.h"

namespace lc3
{
namespace utils
{
    enum class PrintType {
          FATAL_ERROR = 0
        , ERROR
        , WARNING
        , NOTE
        , INFO
        , DEBUG
        , EXTRA
        , SPAM
    };

    class Logger
    {
    protected:
        lc3::utils::IPrinter & printer;
        uint32_t print_level;

    public:
        Logger(IPrinter & printer, uint32_t print_level) : printer(printer), print_level(print_level) {}

        template<typename ... Args>
        void printf(PrintType level, bool bold, std::string const & format, Args ... args) const;
        void newline(void) const { printer.newline(); }
        void print(std::string const & str) { printer.print(str); }
    };

    class AssemblerLogger : public Logger
    {
    public:
        using Logger::Logger;

        AssemblerLogger(IPrinter & printer, uint32_t print_level, std::string const & filename) :
            Logger(printer, print_level), filename(filename) {}

        template<typename ... Args>
        void asmPrintf(PrintType level, lc3::core::asmbl::StatementToken const & token, std::string const & format,
            Args ... args) const;
        template<typename ... Args>
        void asmPrintf(PrintType level, uint32_t col, uint32_t len, lc3::core::asmbl::StatementToken const & token,
            std::string const & format, Args ... args) const;

        std::string filename;
        std::vector<std::string> asm_blob;
    };
};
};

template<typename ... Args>
void lc3::utils::Logger::printf(lc3::utils::PrintType type, bool bold, std::string const & format, Args ... args) const
{
    lc3::utils::PrintColor color = lc3::utils::PrintColor::RESET;
    std::string label = "";

    if(static_cast<uint32_t>(type) <= print_level) {
        switch(type) {
            case PrintType::ERROR:
                color = lc3::utils::PrintColor::RED;
                label = "error";
                break;

            case PrintType::WARNING:
                color = lc3::utils::PrintColor::YELLOW;
                label = "warning";
                break;

            case PrintType::NOTE:
                color = lc3::utils::PrintColor::GRAY;
                label = "note";
                break;

            case PrintType::INFO:
                color = lc3::utils::PrintColor::GREEN;
                label = "info";
                break;

            case PrintType::DEBUG:
                color = lc3::utils::PrintColor::MAGENTA;
                label = "debug";
                break;

            case PrintType::EXTRA:
                color = lc3::utils::PrintColor::BLUE;
                label = "extra";
                break;

            case PrintType::SPAM:
                label = "spam";

            default: break;
        }

        printer.setColor(lc3::utils::PrintColor::BOLD);
        printer.setColor(color);
        printer.print(lc3::utils::ssprintf("%s: ", label.c_str()));
        printer.setColor(lc3::utils::PrintColor::RESET);

        if(bold) {
            printer.setColor(lc3::utils::PrintColor::BOLD);
        }

        printer.print(lc3::utils::ssprintf(format, args...));
        printer.setColor(lc3::utils::PrintColor::RESET);

        printer.newline();
    }
}

template<typename ... Args>
void lc3::utils::AssemblerLogger::asmPrintf(lc3::utils::PrintType level,
    lc3::core::asmbl::StatementToken const & token, std::string const & format, Args ... args) const
{
    asmPrintf(level, token.col, token.len, token, format, args...);
}

template<typename ... Args>
void lc3::utils::AssemblerLogger::asmPrintf(lc3::utils::PrintType level, uint32_t col_num, uint32_t len,
    lc3::core::asmbl::StatementToken const & token, std::string const & format, Args ... args) const
{
    printer.setColor(lc3::utils::PrintColor::BOLD);
    printer.print(lc3::utils::ssprintf("%s:%d:%d: ", filename.c_str(), token.row + 1, col_num + 1));
 
    printf(level, true, format, args...);
    printer.print(lc3::utils::ssprintf("%s", token.line.c_str()));
    printer.newline();
 
    printer.setColor(lc3::utils::PrintColor::BOLD);
    printer.setColor(lc3::utils::PrintColor::GREEN);
 
    for(uint32_t i = 0; i < col_num; i++) {
        printer.print(" ");
    }
    printer.print("^");
 
    for(uint32_t i = 0; i < len - 1; i++) {
        printer.print("~");
    }
 
    printer.setColor(lc3::utils::PrintColor::RESET);
    printer.newline();
}

#endif
