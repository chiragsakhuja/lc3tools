#ifndef ASSEMBLER_H
#define ASSEMBLER_H

class Assembler
{
public:
    static Assembler& getInstance();
    static bool assembleProgram(const std::string& filename, const Token *program, const std::map<std::string, int>& symbolTable);

private:
    static std::vector<std::string> fileBuffer;

    Assembler();
    static bool assembleInstruction(const std::string& filename, const Token *state);
    static bool processStatement(const std::string& filename, const Token *state);

    // cannot duplicate the singleton
    Assembler(Assembler const&)      = delete;
    void operator=(Assembler const&) = delete;
};

#endif
