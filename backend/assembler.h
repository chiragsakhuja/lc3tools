/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <sstream>
#include <vector>

#include "instruction_encoder.h"
#include "logger.h"
#include "optional.h"
#include "printer.h"
#include "tokenizer.h"

namespace lc3
{
namespace core
{
    class Assembler
    {
    public:
        Assembler(lc3::utils::IPrinter & printer, uint32_t print_level) : logger(printer, print_level),
            encoder(logger) {}
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = delete;
        ~Assembler(void) = default;

        std::stringstream assemble(std::istream & buffer);
        void setFilename(std::string const & filename) { logger.setFilename(filename); }

    private:
        std::vector<std::string> file_buffer;
        lc3::utils::AssemblerLogger logger;

        asmbl::InstructionEncoder encoder;

        std::vector<asmbl::StatementNew> buildStatements(std::istream & buffer);
        asmbl::StatementNew buildStatement(std::vector<asmbl::Token> const & tokens);
        void setStatementPCField(std::vector<asmbl::StatementNew> & statements);
        std::pair<bool, SymbolTable> buildSymbolTable(std::vector<asmbl::StatementNew> const & statements);
        std::pair<bool, std::vector<MemEntry>> buildMachineCode(std::vector<asmbl::StatementNew> const & statements,
            SymbolTable const & symbols);

    };
};
};

#endif
