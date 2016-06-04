#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>
#include <map>
#include <string>

#include "../utils/printer.h"
#include "logger.h"
#include "instruction_encoder.h"

class Assembler
{
public:
    bool assembleProgram(std::string const & filename, Token * program);

    Assembler(bool log_enable, utils::Printer const & printer, std::map<std::string, int> & symbol_table);
    Assembler(Assembler const &) = default;
    Assembler & operator=(Assembler const &) = default;

private:
#ifdef _ASSEMBLER_TEST
    FRIEND_TEST(AssemblerSimple, SingleDataProcessingInstruction);
#endif

    std::vector<std::string> file_buffer;
    int sectionStart;
    AssemblerLogger * logger;
    bool log_enable;
    InstructionEncoder * encoder;
    std::map<std::string, int> & symbol_table;

    void processOperands(std::string const & filename, Token * operands);
    bool processInstruction(std::string const & filename, Token const * inst, uint32_t & encoded_instruction) const;
    bool processTokens(std::string const & filename, Token * program, Token *& program_start);
    bool processPseudo(std::string const & filename, Token const * pseudo);
    bool getOrig(std::string const & filename, Token const * orig, int & new_orig);
};

#endif
