#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <map>
#include <string>
#include <vector>

#include "instruction_encoder.h"
#include "logger.h"
#include "../common/printer.h"

namespace core
{
    class Assembler
    {
    public:
        Assembler(bool log_enable, utils::Printer & printer);
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = default;
        ~Assembler(void);

        void assembleProgram(std::string const & filename, Token * program,
            std::map<std::string, uint32_t> & labels);
        void genObjectFile(char const * filename);

    private:
#ifdef _ASSEMBLER_TEST
        FRIEND_TEST(AssemblerSimple, SingleDataProcessingInstruction);
#endif
        std::vector<std::string> file_buffer;
        AssemblerLogger * logger;
        bool log_enable;
        InstructionGenerator * instructions;
        InstructionEncoder * encoder;

        Token * removeNewlineTokens(Token * program);
        void processOperands(std::string const & filename, Token * operands);
        void processInstruction(std::string const & filename, Token const * inst,
            uint32_t & encoded_instruction,
            std::map<std::string, uint32_t> const & labels) const;
        bool processTokens(std::string const & filename, Token * program,
                Token *& program_start);
        bool processPseudo(std::string const & filename, Token const * pseudo);
        bool setOrig(std::string const & filename, Token const * orig, int & new_orig);
    };
};

#endif
