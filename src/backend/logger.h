/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
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
    class Logger
    {
    protected:
        lc3::utils::IPrinter & printer;
        uint32_t print_level;

    public:
        Logger(IPrinter & printer, uint32_t print_level) : printer(printer), print_level(print_level) {}

        lc3::utils::IPrinter & getPrinter(void) const { return printer; }

        template<typename ... Args>
        void printf(PrintType level, bool bold, std::string const & format, Args ... args) const;
        void newline(PrintType level = PrintType::P_ERROR) const {
            if(print_level > static_cast<uint32_t>(level)) { printer.newline(); }
        }
        void print(std::string const & str) {
            if(print_level > static_cast<uint32_t>(PrintType::P_NONE)) { printer.print(str); }
        }
        uint32_t getPrintLevel(void) const { return print_level; }
        void setPrintLevel(uint32_t print_level) { this->print_level = print_level; }
    };

    class AssemblerLogger : public Logger
    {
    public:
        using Logger::Logger;

        AssemblerLogger(IPrinter & printer, uint32_t print_level) : AssemblerLogger(printer, print_level, "") {}
        AssemblerLogger(IPrinter & printer, uint32_t print_level, std::string const & filename) :
            Logger(printer, print_level), filename(filename) {}

        void setFilename(std::string const & filename) { this->filename = filename; }

        template<typename ... Args>
        void asmPrintf(PrintType level, lc3::core::asmbl::Statement const & statement,
            lc3::core::asmbl::StatementPiece const & piece, std::string const & format, Args ... args) const;
        template<typename ... Args>
        void asmPrintf(PrintType level, lc3::core::asmbl::Statement const & statement, std::string const & format,
            Args ... args) const;
        template<typename ... Args>
        void asmPrintf(PrintType level, uint32_t row_num, uint32_t col_num, uint32_t len, std::string const & line,
            std::string const & format, Args ... args) const;

        std::string filename;
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
            case PrintType::P_ERROR:
                color = lc3::utils::PrintColor::RED;
                label = "error";
                break;

            case PrintType::P_WARNING:
                color = lc3::utils::PrintColor::YELLOW;
                label = "warning";
                break;

            case PrintType::P_NOTE:
                color = lc3::utils::PrintColor::GRAY;
                label = "note";
                break;

            case PrintType::P_INFO:
                color = lc3::utils::PrintColor::GREEN;
                label = "info";
                break;

            case PrintType::P_DEBUG:
                color = lc3::utils::PrintColor::MAGENTA;
                label = "debug";
                break;

            case PrintType::P_EXTRA:
                color = lc3::utils::PrintColor::BLUE;
                label = "extra";
                break;

            case PrintType::P_SPAM:
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
    lc3::core::asmbl::Statement const & statement, lc3::core::asmbl::StatementPiece const & piece,
    std::string const & format, Args ... args) const
{
    asmPrintf(level, statement.row, piece.col, piece.len, statement.line, format, args...);
}

template<typename ... Args>
void lc3::utils::AssemblerLogger::asmPrintf(lc3::utils::PrintType level,
    lc3::core::asmbl::Statement const & statement, std::string const & format, Args ... args) const
{
    asmPrintf(level, statement.row, 0, (uint32_t) statement.line.size(), statement.line, format, args...);
}

template<typename ... Args>
void lc3::utils::AssemblerLogger::asmPrintf(lc3::utils::PrintType level, uint32_t row_num, uint32_t col_num,
    uint32_t len, std::string const & line, std::string const & format, Args ... args) const
{
    if(static_cast<uint32_t>(level) > print_level) { return; }

    printer.setColor(lc3::utils::PrintColor::BOLD);
    printer.print(lc3::utils::ssprintf("%s:%d:%d: ", filename.c_str(), row_num + 1, col_num + 1));

    printf(level, true, format, args...);
    printer.print(lc3::utils::ssprintf("%s", line.c_str()));
    printer.newline();

    printer.setColor(lc3::utils::PrintColor::BOLD);
    printer.setColor(lc3::utils::PrintColor::GREEN);

    for(uint32_t i = 0; i < col_num; i++) {
        printer.print(" ");
    }
    printer.print("^");

    if(len > 0) {
        for(uint32_t i = 0; i < len - 1; i++) {
            printer.print("~");
        }
    }

    printer.setColor(lc3::utils::PrintColor::RESET);
    printer.newline();
}

#endif
