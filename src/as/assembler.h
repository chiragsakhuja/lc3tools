#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>
#include <map>
#include <string>

#include "../utils/printer.h"
#include "assembler_printer.h"
#include "instruction_encoder.h"

class Assembler
{
public:
    bool assembleProgram(  bool print_enable, Printer const & printer, std::string const & filename, Token * program
                         , std::map<std::string, int> & symbol_table);

    Assembler(void);
    Assembler(Assembler const &) = default;
    Assembler & operator=(Assembler const &) = default;

private:
#ifdef _ASSEMBLER_TEST
    FRIEND_TEST(AssemblerSimple, SingleDataProcessingInstruction);
#endif

    std::vector<std::string> file_buffer;
    int sectionStart;

    void processOperands(Token * operands);
    bool processInstruction(  bool print_enable, AssemblerPrinter const & printer, InstructionEncoder & encoder
                            , std::string const & filename, Token const * inst, std::map<std::string, int> const & symbol_table
                            , uint32_t & encoded_instruction) const;
    bool processTokens(  bool print_enable, AssemblerPrinter const & printer, InstructionEncoder & encoder
                       , std::string const & filename, Token * program, std::map<std::string, int> & symbol_table
                       , Token *& program_start);
    bool processPseudo(  bool print_enable, AssemblerPrinter const & printer, std::string const & filename
                       , Token const * pseudo);
    bool getOrig(  bool print_enable, AssemblerPrinter const & printer, std::string const & filename, Token const * orig
                 , int & new_orig);
};

#endif
