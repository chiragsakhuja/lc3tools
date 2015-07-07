#include <string>

#include "tokens.h"
#include "parser.hpp"

Token::Token()
{
    this->numOperands = 0;
    this->pc = 0;
    this->rowNum = 0;
    this->colNum = 0;
    this->length = 0;
    this->opers = nullptr;
    this->next = nullptr;
}

Token::Token(const std::string& str) : Token()
{
    this->str = str;
    this->type = STRING;
}

Token::Token(int num) : Token()
{
    this->num = num;
    this->type = NUM;
}

bool Token::checkPseudoType(const std::string& pseudo) const
{
    return type == PSEUDO && str == pseudo;
}
