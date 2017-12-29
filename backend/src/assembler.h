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
        // TODO: make sure program is not leaked
        ~Assembler(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename);

    private:
        std::vector<std::string> file_buffer;
        utils::IPrinter & printer;

        bool log_enable;
        InstructionEncoder encoder;

        std::vector<utils::Statement> assembleChain(Token * program,
            std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);
        Token * firstPass(Token * program, std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);
        Token * removeNewlineTokens(Token * program);
        void toLower(Token * token_chain);
        void separateLabels(Token * program, AssemblerLogger & logger);
        Token * findOrig(Token * program, AssemblerLogger & logger);
        void processStatements(Token * program, AssemblerLogger & logger);
        void processInstOperands(Token * inst);
        void processStringzOperands(Token * stringz);
        void saveSymbols(Token * program, std::map<std::string, uint32_t> & labels, AssemblerLogger & logger);

        std::vector<utils::Statement> secondPass(Token * program, std::map<std::string, uint32_t> symbols,
            AssemblerLogger & logger);
        uint32_t encodeInstruction(Token * program, std::map<std::string, uint32_t> symbols, AssemblerLogger & logger);
        std::vector<utils::Statement> encodePseudo(Token * program, std::map<std::string, uint32_t> symbols,
            AssemblerLogger & logger);
        void processInstruction(std::string const & filename, Token const * inst, AssemblerLogger & logger);
        void processPseudo(std::string const & filename, Token const * inst, std::map<std::string, uint32_t> symbols,
            AssemblerLogger & logger);

        std::vector<std::string> readFile(std::string const & filename);
    };
};

#endif
