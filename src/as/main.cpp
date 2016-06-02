#include <cstdio>
#include <map>
#include <string>

#include "tokens.h"
#include "utils/printer.h"
#include "utils/console_printer.h"
#include "assembler.h"
#include "parser.hpp"

#include "paths.h"

void genObjectFile(char const * filename, utils::Printer const & printer, std::map<std::string, int> & symbol_table);

extern FILE *yyin;
extern int yyparse(void);
extern Token *root;
extern int row_num, col_num;

int main(int argc, char *argv[])
{
    utils::Printer const * printer = new utils::ConsolePrinter();
    std::map<std::string, int> symbol_table;

    if(argc < 2) {
        printer->printf(utils::PrintType::ERROR, "usage: %s file [file ...]", argv[0]);
    } else {
        for(int i = 1; i < argc; i++) {
            genObjectFile(argv[i], *printer, symbol_table);
        }
    }

    delete printer;

    return 0;
}

void genObjectFile(char const * filename, utils::Printer const & printer, std::map<std::string, int> & symbol_table)
{
    Assembler as;

    if((yyin = fopen(filename, "r")) == nullptr) {
         printer.printf(utils::PrintType::WARNING, "Skipping file %s ...", filename);
    } else {
        row_num = 0; col_num = 0;
        yyparse();
        as.assembleProgram(true, printer, filename, root, symbol_table);

        fclose(yyin);
    }
}
