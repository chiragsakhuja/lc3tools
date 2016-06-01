#include <string>

#include "tokens.h"
#include "parser.hpp"

Token::Token(void)
{
    this->num_operands = 0;
    this->pc = 0;
    this->row_num = 0;
    this->col_num = 0;
    this->length = 0;
    this->opers = nullptr;
    this->next = nullptr;
}

Token::Token(const std::string & str) : Token()
{
    this->str = str;
    this->type = STRING;
}

Token::Token(int num) : Token()
{
    this->num = num;
    this->type = NUM;
}

bool Token::checkPseudoType(std::string const & pseudo) const
{
    return type == PSEUDO && str == pseudo;
}
