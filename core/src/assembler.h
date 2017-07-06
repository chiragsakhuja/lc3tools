#ifndef ASSEMBLER_H
#define ASSEMBLER_H

namespace core
{
    class Assembler
    {
    public:
        Assembler(bool log_enable, utils::IPrinter & printer) : printer(printer), log_enable(log_enable) {}
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = default;
        ~Assembler(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename);

    private:
        std::vector<std::string> file_buffer;
        utils::IPrinter & printer;

        bool log_enable;
        InstructionEncoder encoder;

        Token * removeNewlineTokens(Token * program);
        void separateLabels(Token * program, AssemblerLogger & logger);
        void toLower(Token * token_chain);
        Token * findOrig(Token * program, AssemblerLogger & logger);
        Token * firstPass(Token * program, std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);
        std::vector<utils::ObjectFileStatement> assembleChain(Token * program,
            std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);
        std::vector<std::string> readFile(std::string const & filename);
        void processInstOperands(Token * inst);
        void processStringzOperands(Token * stringz);
        void processStatements(Token * program);
        void saveSymbols(Token * program, std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);
        /*
        std::vector<std::string> readFile(std::string const & filename);
        std::vector<utils::ObjectFileStatement> assembleChain(Token * program,
            std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);
        Token * firstPass(Token * program,
            std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);

        Token * removeNewlineTokens(Token * program);
        void processOperands(std::string const & filename, Token * operands);
        void processInstruction(std::string const & filename, Token const * inst,
            uint32_t & encoded_instruction,
            std::map<std::string, uint32_t> const & labels) const;
        void processPseudo(std::string const & filename, Token const * inst,
            std::vector<uint32_t> & object_file,
            std::map<std::string, uint32_t> const & labels) const;
        bool processTokens(std::string const & filename, Token * program,
            std::map<std::string, uint32_t> & labels, Token *& program_start);
        bool setOrig(std::string const & filename, Token const * orig, uint32_t & new_orig);
        void separateLabels(std::string const & filename, Token * program);
        bool findFirstOrig(std::string const & filename, Token * program, Token *& program_start,
                uint32_t & cur_orig);
        void processOperands(Token * inst);
        */
    };
};

#endif
