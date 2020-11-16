/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <memory>
#include <sstream>
#include <vector>

#include "encoder.h"
#include "logger.h"
#include "printer.h"
#include "tokenizer.h"
#include "utils.h"

namespace lc3
{
namespace core
{
    class Assembler
    {
    public:
        Assembler(lc3::utils::IPrinter & printer, uint32_t print_level, bool enable_liberal_asm)
            : logger(printer, print_level), enable_liberal_asm(enable_liberal_asm),
              encoder(logger, enable_liberal_asm) {}
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = delete;
        ~Assembler(void) = default;

        std::shared_ptr<std::stringstream> assemble(std::istream & buffer);
        void setFilename(std::string const & filename) { logger.setFilename(filename); }

        void setLiberalAsm(bool enable_liberal_asm);

    private:
        std::vector<std::string> file_buffer;
        lc3::utils::AssemblerLogger logger;
        bool enable_liberal_asm;

        asmbl::Encoder encoder;

        std::vector<asmbl::Statement> buildStatements(std::istream & buffer);
        asmbl::Statement buildStatement(std::vector<asmbl::Token> const & tokens);
        void setStatementPCField(std::vector<asmbl::Statement> & statements);
        std::pair<bool, SymbolTable> buildSymbolTable(std::vector<asmbl::Statement> const & statements);
        std::pair<bool, std::vector<MemLocation>> buildMachineCode(std::vector<asmbl::Statement> const & statements,
            SymbolTable const & symbols);
    };
};
};

#endif
