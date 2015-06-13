#include "tokens.h"
#include "parser.hpp"

Token::Token()
{
    this->rowNum = 0;
    this->colNum = 0;
    this->length = 0;
    this->args = nullptr;
    this->next = nullptr;
}

Token::Token(std::string *str, int type) : Token()
{
    this->data.str = str;
    this->type = type;
}

Token::Token(int num, int type) : Token()
{
    this->data.num = num;
    this->type = type;
}

Token::~Token()
{
    if(type == STRING && data.str != nullptr) {
        delete data.str;
    }
}

