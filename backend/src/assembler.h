#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>

#include "instruction_encoder.h"
#include "tokenizer.h"
#include "printer.h"

namespace lc3
{
namespace core
{
    class Assembler
    {
    public:
        Assembler(lc3::utils::IPrinter & printer, uint32_t print_level) : printer(printer), print_level(print_level) {}
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = default;
        ~Assembler(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename);
        void convertBin(std::string const & bin_filename, std::string const & obj_filename);

    private:
        std::vector<std::string> file_buffer;
        lc3::utils::IPrinter & printer;

        uint32_t print_level;
        asmbl::InstructionEncoder encoder;

        SymbolTable firstPass(std::vector<asmbl::Statement> const & statements,
            lc3::utils::AssemblerLogger & logger);
        std::vector<MemEntry> secondPass(std::vector<asmbl::Statement> const & statements,
            SymbolTable const & symbol_table, lc3::utils::AssemblerLogger & logger, bool & success);
        void writeFile(std::vector<MemEntry> const & obj_blob, std::string const & obj_filename,
            lc3::utils::Logger & logger);

        asmbl::Statement makeStatement(std::vector<asmbl::Token> const & tokens);
        void markRegAndPseudoTokens(std::vector<asmbl::StatementToken> & tokens);
        void markInstTokens(std::vector<asmbl::StatementToken> & tokens);
        void markLabelTokens(std::vector<asmbl::StatementToken> & tokens);
        asmbl::Statement makeStatementFromTokens(std::vector<asmbl::StatementToken> & tokens);
        void markPC(std::vector<asmbl::Statement> & statements, lc3::utils::AssemblerLogger & logger);
        uint32_t encodeInstruction(asmbl::Statement const & statement, SymbolTable const & symbol_table,
            lc3::utils::AssemblerLogger & logger, bool & success);
        uint32_t encodePseudo(asmbl::Statement const & statement, SymbolTable const & symbol_table,
            lc3::utils::AssemblerLogger & logger, bool & success);

        bool checkIfValidPseudoToken(asmbl::StatementToken const & tok, std::string const & check);
        bool checkIfValidPseudoStatement(asmbl::Statement const & state, std::string const & check,
            lc3::utils::AssemblerLogger & logger, bool log_enable);
    };
};
};

#endif
