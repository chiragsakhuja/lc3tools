%{

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

#include "../tokens.h"
#include "parser.hpp"

int stringToInt(char const * str);
void yyerror(char const *);
Token * makeToken(char const * tok, int type);

int row_num = 0, col_num = 0;

%}

%option noyywrap case-insensitive pointer

LE      [\r\n]
NLE     [^\r\n]
WS      [ \t]
NWS     [^ \t]
NDEL    [^ ,:.\r\n\t]
BDIG    [0-1]
DIG     [0-9]
HDIG    [0-9a-f]

%%

{LE}{1,2}               { col_num = 0; row_num += 1; if(strlen(yytext) == 2 && yytext[0] == yytext[1]) { row_num += 1; } return NEWLINE; }

[,]                     { col_num += strlen(yytext); return COMMA; }
[.]                     { col_num += strlen(yytext); return DOT  ; }
[:]                     { col_num += strlen(yytext); return COLON; }

{WS}+                   { col_num += strlen(yytext); }
(;|("//")){NLE}*        { col_num += strlen(yytext); }

0?b-?{BDIG}+            { yylval.tok = makeToken(yytext, NUM)   ; col_num += strlen(yytext); return NUM   ; }
#?-?{DIG}+              { yylval.tok = makeToken(yytext, NUM)   ; col_num += strlen(yytext); return NUM   ; }
0?x-?{HDIG}+            { yylval.tok = makeToken(yytext, NUM)   ; col_num += strlen(yytext); return NUM   ; }
{NDEL}+|['"]{NLE}*['"]  { yylval.tok = makeToken(yytext, STRING); col_num += strlen(yytext); return STRING; }

%%

Token * makeToken(char const * tok, int type)
{
    Token * ret = nullptr;

    if(type == STRING) {
        std::string tokStr(tok);
        std::transform(tokStr.begin(), tokStr.end(), tokStr.begin(), ::tolower);
        ret = new Token(tokStr);
    } else {
        ret = new Token(stringToInt(tok));
    }

    ret->row_num = row_num;
    ret->col_num = col_num;
    ret->length = strlen(tok);

    return ret;
}

int stringToInt(char const * str)
{
    if(str[0] == '0' && str[1] != '\0') { str += 1; }

    if(str[0] == 'b' || str[0] == 'x' || str[0] == '#') {
        std::string conv = std::string(str).substr(1);
        switch(str[0]) {
            case 'b': return std::stoi(conv, 0, 2) ;
            case 'x': return std::stoi(conv, 0, 16);
            case '#': return std::stoi(conv)       ;
            default : return std::stoi(str)        ;
        }
    } else {
        return std::stoi(str);
    }
}
