#ifndef ASM_TYPES_H
#define ASM_TYPES_H

#include <string>
#include <vector>
#include <iostream>

#include "optional.h"

namespace lc3
{
namespace core
{
namespace asmbl
{

    struct Token
    {
        enum class Type
        {
              NUM = 0
            , STRING
            , EOL
            , INVALID
        } type;

        Token(void);

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
        std::vector<StatementToken> invalid_operands;

        std::string line;
    };

    struct StatementPiece
    {
        enum class Type
        {
              INST = 0
            , PSEUDO
            , LABEL
            , REG
            , STRING
            , NUM
            , INVALID
        } type;

        std::string str;
        uint32_t num;

        StatementPiece(void) : type(Type::INVALID) {}
        StatementPiece(uint32_t num) : type(Type::NUM), num(num) {}
        StatementPiece(std::string const & str, Type type) : type(type), str(str)
        {
#ifdef _ENABLE_DEBUG
            assert(type != Type::NUM);
#endif
        }
    };

    struct StatementNew
    {
        optional<StatementPiece> label;
        optional<StatementPiece> base;
        std::vector<StatementPiece> operands;

        uint64_t pc;
        std::string line;
    };
};
};
};

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementToken const & x);
std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::Statement const & x);
std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementPiece const & piece);
std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementNew const & statement);

#endif
