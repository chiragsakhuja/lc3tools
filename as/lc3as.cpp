#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>

#include "tokens.h"
#include "lc3.tab.h"

template<typename... Args>
void printWarning(const char *format, Args... args);
std::string argsToString(Token *tok);
void printProgram(Token *head);
void bufferFile(const char *filename, std::vector<std::string> &buffer);
void assembleProgram(Token *program);
void genObjectFile(const char *filename);
extern FILE *yyin;
extern int yyparse(void);
extern Token *root;

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " file [file ...]\n";
    } else {
        for(int i = 1; i < argc; i++) {
            genObjectFile(argv[i]);
        }
    }

    return 0;
}

void bufferFile(const char *filename, std::vector<std::string> &buffer)
{
    std::ifstream file(filename);

    if(file.is_open()) {
        std::string line;

        buffer.clear();
        while(std::getline(file, line)) {
            buffer.push_back(line);
        }

        file.close();
    }
}

void assembleProgram(Token *program)
{

}

void genObjectFile(const char *filename)
{
    if((yyin = fopen(filename, "r")) == nullptr) {
        printWarning("Skipping file %s ...", filename);
    } else {
        std::vector<std::string> buffer;

        bufferFile(filename, buffer);
        yyparse();
        assembleProgram(root);
        // printProgram(root);

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

        Token *cur_arg = head->args;
        while(cur_arg != nullptr) {
            printf("%s (%d,%d)  ", argsToString(cur_arg).c_str(), cur_arg->rowNum, cur_arg->colNum);
            cur_arg = cur_arg->next;
        }

        printf("\n");

        head = head->next;
    }
}

template<typename... Args>
void printWarning(const char *format, Args... args)
{
    std::cout << "Warning: ";
    std::printf(format, args...);
    std::cout << std::endl;
}
