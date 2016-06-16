#ifdef _ENABLE_DEBUG
#include <iostream>
#endif

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

#ifdef _ENABLE_DEBUG
void Token::print(std::ostream & out, int indent_level) const
{
    if(indent_level != 0) {
        for(int i = 0; i < indent_level; i += 1) {
            out << "   ";
        }
        out << "|- ";
    }

    if(type == STRING || type == PSEUDO || type == LABEL || type == INST) {
        out << str;
    } else if(type == NUM) {
        out << num;
    } else if(type == NEWLINE) {
        out << "NEWLINE";
    } else if(type == COLON) {
        out << ":";
    } else if(type == COMMA) {
        out << ".";
    } else {
        out << "Unknown token";
    }

    out << " (" << (row_num + 1) << ", " << (col_num + 1) << ", " << length << ")" << std::endl;
    Token * cur_oper = opers;
    while(cur_oper != nullptr) {
        cur_oper->print(out, indent_level + 1);
        cur_oper = cur_oper->next;
    }
}

std::ostream & operator<<(std::ostream & out, Token const & x)
{
    x.print(out, 0);
    return out;
}
#endif
