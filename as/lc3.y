%{

#include <cstdio>
#include <cstdlib>
#include <string>
#include "tokens.h"

Token *append(Token *head, Token *list);
std::string argsToString(Token *tok);
void printProgram(Token *head);

extern int yylex();
void yyerror(const char *);

Token *root = nullptr;

%}

%union {
    Token *tok;
}

%token <tok> NUM STRING

%token NEWLINE COLON COMMA DOT
%token PSEUDO LABEL INST

%type <tok> arg arglist inst pseudo label state program

%start tree

%%

arg     : STRING                { $$ = $1; }
        | NUM                   { $$ = $1; }
        ;

arglist : arg COMMA arglist     { $$ = append($1, $3); }
        | arg arglist           { $$ = append($1, $2); }
        | arg COMMA             { $$ = $1; }
        | arg                   { $$ = $1; }
        ;

inst    : STRING arglist        { $1->type = INST; $1->args = $2; $$ = $1; }
        | STRING                { $1->type = INST; $1->args = nullptr; $$ = $1; }
        ;

pseudo  : DOT inst              { $2->type = PSEUDO; $$ = $2; }
        ;

label   : STRING COLON NEWLINE  { $1->type = LABEL; $$ = $1; }
        | STRING COLON          { $1->type = LABEL; $$ = $1; }
        ;

state   : inst NEWLINE          { $$ = $1; }
        | pseudo NEWLINE        { $$ = $1; }
        | label                 { $$ = $1; }
        ;

program : state program         { $$ = append($1, $2); }
        | state                 { $$ = $1; }
        ;

tree    : program               { root = $1; printProgram($1); }
        ;

%%

void yyerror(const char *str) { printf("Parse Error: %s\n", str); }

Token * append(Token *head, Token *list)
{
    head->next = list;
    return head;
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
        printf("%s (%d): ", argsToString(head).c_str(), head->type);

        Token *cur_reg = head->args;
        while(cur_reg != nullptr) {
            printf("%s ", argsToString(cur_reg).c_str());
            cur_reg = cur_reg->next;
        }

        printf("\n");

        head = head->next;
    }
}
