#include <string>

#include "tokens.h"
#include "parser.hpp"

Token::Token()
{
    this->data.str = nullptr;
    this->numOperands = 0;
    this->pc = 0;
    this->rowNum = 0;
    this->colNum = 0;
    this->length = 0;
    this->args = nullptr;
    this->next = nullptr;
}

Token::Token(std::string *str) : Token()
{
    this->data.str = str;
    this->type = STRING;
}

Token::Token(int num) : Token()
{
    this->data.num = num;
    this->type = NUM;
}

Token::~Token()
{
    if(type == STRING && data.str != nullptr) {
        delete data.str;
        data.str = nullptr;
    }
}

bool Token::checkPseudoType(const std::string& pseudo) const
{
    return type == PSEUDO && (*data.str) == pseudo;
}
