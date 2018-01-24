#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <vector>
#include <iostream>

struct OldToken
{
    enum class TokenType {
          NEWLINE
        , STRING
        , NUM
    } token_type;

    OldToken(void);

    std::string str;
    int num;

    int type;
    int num_opers;
    int pc;
    OldToken * opers;
    OldToken * next;

    OldToken(std::string const & str);
    OldToken(int val);

    // used for errors
    int row_num, col_num;
    int length;

    bool checkPseudoType(std::string const & pseudo) const;

#ifdef _ENABLE_DEBUG
    void print(std::ostream & out, int indent_level) const;
    friend std::ostream & operator<<(std::ostream &, OldToken const &);
#endif
};

std::ostream & operator<<(std::ostream & out, OldToken const & x);

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
    };

    struct Statement
    {
        bool isPseudo(void) const;
        bool isInst(void) const;
        bool hasLabel(void) const;

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
