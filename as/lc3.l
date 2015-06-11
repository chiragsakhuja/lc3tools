%{

#include <cstdio>
#include <string>
#include <cstring>

#include "tokens.h"
#include "lc3.tab.h"

int stringToInt(const char *str);

void yyerror(const char *);

%}

%option noyywrap case-insensitive pointer

LE      [\r\n]
NLE     [^\r\n]
WS      [ \n\r\t]
NWS     [^ \n\r\t]
NDEL    [^ ,:.\n\r\t]
BDIG    [0-1]
DIG     [0-9]
HDIG    [0-9a-f]

%%

{LE}                    { return NEWLINE; }
[,]                     { return COMMA  ; }
[.]                     { return DOT    ; }
[:]                     { return COLON  ; }

{WS}+                   { }
(;|("//")){NLE}*        { }

0?b{BDIG}+              { yylval.tok = new Token(stringToInt(yytext), NUM) ; return NUM ; }
#?{DIG}+                { yylval.tok = new Token(stringToInt(yytext), NUM) ; return NUM ; }
0?x{HDIG}+              { yylval.tok = new Token(stringToInt(yytext), NUM) ; return NUM ; }

{NDEL}+|['"]{NLE}*['"]  { yylval.tok = new Token(new std::string(yytext), STRING) ; return STRING ; }

%%

int stringToInt(const char *str)
{
    if(str[0] == '0') { str++; }

    std::string conv = std::string(str).substr(1);

    switch(str[0]) {
        case 'b': return std::stoi(conv, 0, 2) ; break;
        case 'x': return std::stoi(conv, 0, 16); break;
        case '#': return std::stoi(conv)       ; break;
        default : return std::stoi(str)        ; break;
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        yyin = stdin;
    } else {
        if((yyin = fopen(argv[1], "r")) == NULL) {
            yyin = stdin;
        }
    }

    yyparse();

    fclose(yyin);

    return 0;
}
