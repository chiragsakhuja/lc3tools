#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <string>

#include "instruction.h"
#include "tokens.h"
#include "assembler.h"
#include "parser.hpp"

void genObjectFile(const char *filename, std::map<std::string, int> &symbolTable);

std::string argsToString(Token *tok);
void printProgram(Token *head);

extern FILE *yyin;
extern int yyparse(void);
extern Token *root;
extern int rowNum, colNum;

int main(int argc, char *argv[])
{
    std::map<std::string, int> symbolTable;

    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " file [file ...]\n";
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

    if((yyin = fopen(filename, "r")) == nullptr) {
        // printWarning("Skipping file %s ...", filename);
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

void printProgram(Token *head)
{
    while(head != nullptr) {
        printf("%s (%d,%d): ", argsToString(head).c_str(), head->rowNum, head->colNum);

        Token *curArg = head->args;
        while(curArg != nullptr) {
            printf("%s (%d,%d)  ", argsToString(curArg).c_str(), curArg->rowNum, curArg->colNum);
            curArg = curArg->next;
        }

        printf("\n");

        head = head->next;
    }
}
