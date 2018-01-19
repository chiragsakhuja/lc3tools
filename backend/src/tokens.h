#ifndef TOKENS_H
#define TOKENS_H

#include <string>

struct Token
{
    enum class TokenType {
          NEWLINE
        , STRING
        , NUM
        , COMMENT
    } token_type;

    Token(void);

    std::string str;
    int num;

    int type;
    int num_opers;
    int pc;
    Token * opers;
    Token * next;

    Token(std::string const & str);
    Token(int val);

    // used for errors
    int row_num, col_num;
    int length;

    bool checkPseudoType(std::string const & pseudo) const;

#ifdef _ENABLE_DEBUG
    void print(std::ostream & out, int indent_level) const;
    friend std::ostream & operator<<(std::ostream &, Token const &);
#endif
};

std::ostream & operator<<(std::ostream & out, Token const & x);

#endif
