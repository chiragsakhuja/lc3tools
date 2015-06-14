#ifndef ASSEMBLER_H
#define ASSEMBLER_H

class Assembler
{
public:
    static Assembler& getInstance();
    static bool assembleProgram(const char *filename, Token *program, std::map<std::string, int> &symbolTable);

private:
    static std::vector<std::string> fileBuffer;
    static std::map<std::string, int> opcodeLUT;

    Assembler();
    static bool assembleInstruction(const char *filename, Token *state);
    static bool processStatement(const char *filename, Token *state);

    // cannot duplicate the singleton
    Assembler(Assembler const&)      = delete;
    void operator=(Assembler const&) = delete;
};

#endif
