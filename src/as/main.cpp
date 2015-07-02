#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <string>

#include "tokens.h"
#include "utils/printer.h"
#include "assembler.h"
#include "parser.hpp"

#include "paths.h"

void genObjectFile(const char *filename, std::map<std::string, int> &symbolTable);

extern FILE *yyin;
extern int yyparse(void);
extern Token *root;
extern int rowNum, colNum;

int main(int argc, char *argv[])
{
    Printer& printer = Printer::getInstance();
    std::map<std::string, int> symbolTable;

    if(argc < 2) {
        printer.printfMessage(Printer::ERROR, "usage: %s file [file ...]", argv[0]);
    } else {
        for(int i = 1; i < argc; i++) {
            genObjectFile(argv[i], symbolTable);
        }
    }

    return 0;
}
