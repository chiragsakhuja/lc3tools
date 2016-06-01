#include <cstdio>
#include <map>
#include <string>

#include "tokens.h"
#include "utils/printer.h"
#include "assembler.h"
#include "parser.hpp"

#include "paths.h"

void genObjectFile(char const * filename, std::map<std::string, int> & symbol_table);

extern FILE *yyin;
extern int yyparse(void);
extern Token *root;
extern int row_num, col_num;

int main(int argc, char *argv[])
{
    Printer printer;
    std::map<std::string, int> symbol_table;

    if(argc < 2) {
        printer.printfMessage(Printer::ERROR, "usage: %s file [file ...]", argv[0]);
    } else {
        for(int i = 1; i < argc; i++) {
            genObjectFile(argv[i], symbol_table);
        }
    }

    return 0;
}

void genObjectFile(char const * filename, std::map<std::string, int> & symbol_table)
{
    Assembler as;
    Printer printer;

    if((yyin = fopen(filename, "r")) == nullptr) {
         printer.printfMessage(Printer::WARNING, "Skipping file %s ...", filename);
    } else {
        row_num = 0; col_num = 0;
        yyparse();
        as.assembleProgram(true, printer, filename, root, symbol_table);

        fclose(yyin);
    }
}
