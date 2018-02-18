#ifndef ASM_TYPES_H
#define ASM_TYPES_H

#include <string>
#include <vector>
#include <iostream>

namespace lc3
{
namespace core
{
namespace asmbl
{
    enum class TokenType {
          INVALID = 0
        , NUM
        , STRING
        , EOS

        , INST
        , REG
        , PSEUDO
        , LABEL
    };

    struct Token
    {
        Token(void);

        TokenType type;
        std::string str;
        int32_t num;

        uint32_t row, col, len;
        std::string line;
    };

    struct StatementToken : public Token
    {
        using Token::Token;

        StatementToken(void);
        StatementToken(Token const & that);

        uint32_t lev_dist;
        uint32_t pc;
    };

    struct Statement
    {
        bool isPseudo(void) const;
        bool isInst(void) const;
        bool hasLabel(void) const;
        bool isLabel(void) const;

        StatementToken label;
        StatementToken inst_or_pseudo;
        uint32_t pc;

        std::vector<StatementToken> operands;

        std::string line;
    };
};
};
};

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementToken const & x);
std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::Statement const & x);

#endif
