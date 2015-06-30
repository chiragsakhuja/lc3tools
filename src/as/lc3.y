%{

#include <cstdio>
#include <cstdlib>
#include <string>
#include "tokens.h"

Token *append(Token *head, Token *list);
void countOperands(Token *inst);

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

%type <tok> oper operlist inst pseudo label state program

%start tree

%%

oper     : STRING                { $$ = $1; }
         | NUM                   { $$ = $1; }
         ;

operlist : oper COMMA operlist   { $$ = append($1, $3); }
         | oper operlist         { $$ = append($1, $2); }
         | oper COMMA            { $$ = $1; }
         | oper                  { $$ = $1; }
         ;

inst     : STRING operlist       { $1->type = INST; $1->opers = $2; countOperands($1); $$ = $1; }
         | STRING                { $1->type = INST; $1->opers = nullptr; $1->numOperands = 0; $$ = $1; }
         ;

pseudo   : DOT inst              { $2->type = PSEUDO; $$ = $2; }
         ;

label    : STRING COLON NEWLINE  { $1->type = LABEL; $$ = $1; }
         | STRING COLON          { $1->type = LABEL; $$ = $1; }
         ;

state    : inst NEWLINE          { $$ = $1; }
         | pseudo NEWLINE        { $$ = $1; }
         | label                 { $$ = $1; }
         ;

program  : state program         { $$ = append($1, $2); }
         | state                 { $$ = $1; }
         ;

tree     : program               { root = $1; }
         ;

%%

void yyerror(const char *str) { printf("Parse Error: %s\n", str); }

Token * append(Token *head, Token *list)
{
    head->next = list;
    return head;
}

void countOperands(Token *inst)
{
    Token *cur = inst->opers;
    int count = 0;

    while(cur != nullptr) {
        count++;
        cur = cur->next;
    }

    inst->numOperands = count;
}

