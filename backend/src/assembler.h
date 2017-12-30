#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>

#include "instruction_encoder.h"
#include "printer.h"

namespace lc3::core
{
    class Assembler
    {
    public:
        Assembler(lc3::utils::IPrinter & printer, uint32_t print_level) : printer(printer), print_level(print_level) {}
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = default;
        // TODO: make sure program is not leaked
        ~Assembler(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename);

    private:
        std::vector<std::string> file_buffer;
        lc3::utils::IPrinter & printer;

        uint32_t print_level;
        InstructionEncoder encoder;

        std::vector<lc3::core::Statement> assembleChain(Token * program,
            std::map<std::string, uint32_t> & labels, lc3::utils::AssemblerLogger & logger);
        Token * firstPass(Token * program, std::map<std::string, uint32_t> & labels,
            lc3::utils::AssemblerLogger & logger);
        Token * removeNewlineTokens(Token * program);
        void toLower(Token * token_chain);
        void separateLabels(Token * program, lc3::utils::AssemblerLogger & logger);
        Token * findOrig(Token * program, lc3::utils::AssemblerLogger & logger);
        void processStatements(Token * program, lc3::utils::AssemblerLogger & logger);
        void processInstOperands(Token * inst);
        void processStringzOperands(Token * stringz);
        void saveSymbols(Token * program, std::map<std::string, uint32_t> & labels,
            lc3::utils::AssemblerLogger & logger);

        std::vector<lc3::core::Statement> secondPass(Token * program, std::map<std::string, uint32_t> symbols,
            lc3::utils::AssemblerLogger & logger);
        uint32_t encodeInstruction(Token * program, std::map<std::string, uint32_t> symbols,
            lc3::utils::AssemblerLogger & logger);
        std::vector<lc3::core::Statement> encodePseudo(Token * program, std::map<std::string, uint32_t> symbols,
            lc3::utils::AssemblerLogger & logger);
        void processInstruction(std::string const & filename, Token const * inst,
            lc3::utils::AssemblerLogger & logger);
        void processPseudo(std::string const & filename, Token const * inst,
            std::map<std::string, uint32_t> symbols, lc3::utils::AssemblerLogger & logger);

        std::vector<std::string> readFile(std::string const & filename);
    };
};

#endif
