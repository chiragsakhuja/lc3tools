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

std::string argsToString(Token *tok);

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

void genObjectFile(const char *filename, std::map<std::string, int> &symbolTable)
{
    Assembler& as = Assembler::getInstance();
    Printer& printer = Printer::getInstance();

    if((yyin = fopen(filename, "r")) == nullptr) {
         printer.printfMessage(Printer::WARNING, "Skipping file %s ...", filename);
    } else {
        rowNum = 0; colNum = 0;
        yyparse();
        as.assembleProgram(filename, root, symbolTable);

        fclose(yyin);
    }
}

std::string argsToString(Token *tok)
{
    if(tok->type == NUM) {
        return std::to_string(tok->data.num);
    } else {
        if(tok->data.str != nullptr) {
            return *tok->data.str;
        } else {
            return std::string();
        }
    }
}
