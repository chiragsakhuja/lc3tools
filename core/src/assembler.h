#ifndef ASSEMBLER_H
#define ASSEMBLER_H

namespace core
{
    class Assembler
    {
    public:
        Assembler(bool log_enable, utils::IPrinter & printer);
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = default;
        ~Assembler(void) = default;

        bool assembleProgram(std::string const & filename, Token * program,
            std::map<std::string, uint32_t> & labels, std::vector<uint32_t> & object_file);
        void genObjectFile(std::string const & filename);

    private:
#ifdef _ASSEMBLER_TEST
        FRIEND_TEST(AssemblerSimple, SingleDataProcessingInstruction);
#endif
        std::vector<std::string> file_buffer;
        AssemblerLogger logger;
        bool log_enable;
        InstructionEncoder encoder;

        Token * removeNewlineTokens(Token * program);
        void processOperands(std::string const & filename, Token * operands);
        void processInstruction(std::string const & filename, Token const * inst,
            uint32_t & encoded_instruction,
            std::map<std::string, uint32_t> const & labels) const;
        bool processTokens(std::string const & filename, Token * program,
            std::map<std::string, uint32_t> & labels, Token *& program_start);
        bool processPseudo(std::string const & filename, Token const * pseudo);
        bool setOrig(std::string const & filename, Token const * orig, uint32_t & new_orig);
        bool findFirstOrig(std::string const & filename, Token * program, Token *& program_start,
                uint32_t & cur_orig);
        void processOperands(Token * inst);
    };
};

#endif
