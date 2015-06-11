#include "tokens.h"

Token::Token(std::string *str, int type)
{
    this->data.str = str;
    this->type = type;
    this->args = nullptr;
    this->next = nullptr;
}

Token::Token(int num, int type)
{
    this->data.num = num;
    this->type = type;
    this->args = nullptr;
    this->next = nullptr;
}

Token::~Token()
{
    if(data.str != nullptr) {
        delete data.str;
    }
}

